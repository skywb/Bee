#include "Service.h"
#include "log/mlog.hpp"
#include <mutex>

#include <random>

using namespace Bee;

void Service::SendNack(const size_t package_num) {
	LOG_DEBUG << "Send NACK " << package_num;
	auto buf = std::make_unique<Buffer> ();
	buf->SetPackNum(package_num);
	buf->SetBufferType(BufferType::NACK);
	buf->SetData(nullptr, 0);
	receiver_->SendBufferToService(std::move(buf));
}

void Service::SendPackageTo(std::shared_ptr<Buffer> buf, const UDPEndPoint endpoint) {
	sender_->SendBufferTo(buf, endpoint);
}
Service::Service() :
	socket_(service_), 
	using_process_thread_cnt_(0) {
		arrived_callback_ = std::make_unique<PackageArrivedCallbackDefault> ();
		sended_callback_ = std::make_unique<PackageSendedCallbackDefault> ();
}

Service::~Service() {
	if (!service_.stopped())
		service_.stop();
	for (int i = 0; i < threads_.size(); ++i) {
		if (threads_[i].joinable()) {
			threads_[i].join();
		}
	}
}

void Service::Init() {
	package_control_ = std::make_unique<PackageControl> ();
	recover_manager_ = std::make_unique<RecoverManager> (service_, this);
	receiver_ = std::make_unique<UDPReceiver> (socket_,
		   	std::bind(&Service::ReceivedHandler, this, std::placeholders::_1, std::placeholders::_2));
	sender_ = std::make_unique<UDPSender> (socket_);
}

void Service::SetThreadCount(int thread_count) {
	for (int i = 0; i < thread_count; ++i) {
		threads_.emplace_back([&]() {
				boost::asio::io_service::work work(service_);
				service_.run();
				});
	}
}

void Service::SetLocalAddress(const std::string IP, const short port)  {
	boost::asio::ip::udp::endpoint local_endpoint(boost::asio::ip::address::from_string(IP), port);
	boost::system::error_code error;
	if (socket_.is_open() ) {
		socket_.close();
	}
	socket_.open(local_endpoint.protocol(), error);
	if (error) {
		LOG_WARN << "socket open error : " << error.message();
	}
	//socket_.set_option(boost::asio::ip::udp::socket::send_buffer_size(1024*800));
	socket_.set_option(boost::asio::ip::udp::socket::receive_buffer_size(1024*2000));
	socket_.bind(local_endpoint, error);
	if (error) {
		LOG_WARN << "bind error : " << error.message();
	}
	LOG_INFO << "listen IP: " << IP << " Port: " << port;
}

void Service::Run() {
	receiver_->AsyncReceive();
}

void Service::Stop() {
	service_.stop();
}

void Service::SendPackage(std::unique_ptr<Package> package, BeeCallback callback) {
	std::shared_ptr<Package> pack(package.release());
	service_.post(std::bind(&Service::DoSendPackage, this, pack, callback));
}

void Service::DoSendPackage(std::shared_ptr<Package> package, BeeCallback callback) {
	auto buffers = std::move(package_control_->SplitPackage(package));
	for (int i = 0; i < buffers.size(); ++i) {
		if (i == buffers.size() -1) {
			AddBufferToSendQue(buffers[i], callback);
		}
		else {
			AddBufferToSendQue(buffers[i]);
		}
		recover_manager_->AddPackRecord(buffers[i]);
	}
}

void Service::ReceivedHandler(std::unique_ptr<Buffer> buffer, UDPEndPoint endpoint) {
	//std::random_device rd;
	//std::uniform_int_distribution<> dis(0,9);
	switch (buffer->GetBufferHeader().type) {
		case BufferType::DATA:
			//if (dis(rd) < 2) {
			//	break;
			//}
			OnDataRecived(std::move(buffer));
			break;
		case BufferType::NACK:
			//LOG_INFO << "NACK " << buffer->GetBufferHeader().pack_num;
			OnNACKRecived(std::move(buffer), endpoint);
			break;
		case BufferType::BUFFER_NOT_FOUND:
			LOG_INFO << "BUFFER_NOT_FOUND";
			OnNotFoundPackRecived(std::move(buffer));
			break;
		case BufferType::HEARTBEAT:
			LOG_INFO << "Recv HEARTBEAT";
			OnHeartBeatReceived(std::move(buffer), endpoint);
			break;
		case BufferType::SYNC_HEATBEAT:
			OnSYNCHeartBeatReceived(std::move(buffer), endpoint);
			break;
		case BufferType::REQUEST:
			LOG_INFO << "REQUEST not implease";
			break;
		default:
			LOG_ERROR << "not found this type";
	}
}

size_t Service::GetRTT() {
	return recover_manager_->GetRTT();
}

void Service::OnDataRecived(std::unique_ptr<Buffer> buf) {
	bool is_effective_buffer = recover_manager_->PackageArrived(buf->GetBufferHeader().pack_num);
	if (is_effective_buffer) {
		AddBufferToQue(std::move(buf));
	}
}

void Service::AddBufferToQue(std::unique_ptr<Buffer> buf) {
	std::unique_lock<std::mutex> lock(mutex_buffer_que_);
	buffer_que_.push(std::move(buf));
	lock.unlock();
	AsyncProcessBuffers();
}

void Service::AsyncProcessBuffers() {
	std::unique_lock<std::mutex> lock(mutex_using_process_thread_cnt_);
	if (using_process_thread_cnt_ >= 1) return;
	//if (using_process_thread_cnt_ != 0 && using_process_thread_cnt_ < threads_.size() - 2)  {
	//	return;
	//} else {
	//	using_process_thread_cnt_++;
	//}
	lock.unlock();
	while (true) {
		std::unique_ptr<Buffer> buf = std::move(GetBufferFromQue());
		if (!buf) { 
			break;
		}
		std::shared_ptr<Buffer> buf_shared = std::move(buf);
		if (transpond_) {
			sender_->SendBuffer(buf_shared);
		}
		if (buf_shared->GetBufferHeader().begin == buf_shared->GetBufferHeader().pack_num) {
			recover_manager_->WaitPackage(buf_shared->GetBufferHeader().begin, 
					buf_shared->GetBufferHeader().begin+buf_shared->GetBufferHeader().count-1);
		}
		package_control_->OnReceivedBuffer(buf_shared);
	}
	lock.lock();	
	using_process_thread_cnt_--;
	lock.unlock();
}

std::unique_ptr<Buffer> Service::GetBufferFromQue() {
	std::unique_lock<std::mutex> lock(mutex_buffer_que_);
	if (buffer_que_.empty()) {
		lock.unlock();
		return nullptr;
	}
	auto p = std::move(buffer_que_.front());
	buffer_que_.pop();
	lock.unlock();
	return std::move(p);
}

void Service::OnNACKRecived(std::unique_ptr<Buffer> buf, UDPEndPoint endpoint) {
	recover_manager_->NACKReceived(buf->GetBufferHeader().pack_num, endpoint);
}

void Service::OnNotFoundPackRecived(std::unique_ptr<Buffer> buf) {
	package_control_->OnBufferNotFound(buf->GetBufferHeader().pack_num);
}

void Service::OnHeartBeatReceived(std::unique_ptr<Buffer> buf, const UDPEndPoint endpoint) {
	if (!sender_->Heartbeat(endpoint)) {
		LOG_INFO << "new client IP: " << endpoint.IP << " Port: " << endpoint.port;
		sender_->AddClient(endpoint);
	}
	LOG_DEBUG << sender_->GetHeartRate() << "    " << buf->GetBufferHeader().heartbeat_rate;
	if (sender_->GetHeartRate() != buf->GetBufferHeader().heartbeat_rate) {
		auto msg = std::make_unique<Buffer>();
		msg->SetBufferType(BufferType::SYNC_HEATBEAT);
		msg->SetHeartRate(sender_->GetHeartRate());
		sender_->SendBufferTo(std::move(msg), endpoint);
	}
}

void Service::OnSYNCHeartBeatReceived(std::unique_ptr<Buffer> buf, const UDPEndPoint endpoint) {
	receiver_->SetHeartbeatRate(buf->GetBufferHeader().heartbeat_rate);
}

void Service::SetBufferOutTime(int ms) {

}

void Service::SetBufferMaxCount(const size_t size) {
	recover_manager_->SetHistoryMaxSize(size);
}

void Service::AddClient(const std::string IP, const short port) {
	sender_->AddClient(UDPEndPoint{IP,port});
}

void Service::RemoveClient(const std::string IP, const short port) {
	sender_->RemoveClient(UDPEndPoint{IP,port});
}

void Service::ConnectService(const std::string IP, const short port) {
	receiver_->AddService(UDPEndPoint{IP,port});
}
 
void Service::DeConnectService(const std::string IP, const short port) {
	receiver_->RemoveService(UDPEndPoint{IP,port});
}

bool Service::Request(std::unique_ptr<Package> package, UDPEndPoint endpoint) {
	if (package->GetSize() > Package::GetMaxSizeOfNotSplit()) return false;
	auto buf = std::make_unique<Buffer> ();
	buf->SetData(package->GetData(), package->GetSize());
	buf->SetBufferType(BufferType::REQUEST);
	receiver_->SendBufferToService(std::move(buf));
	return true;
}

void Service::SetTranspond(bool transpond) {

}


void Service::AddBufferToSendQue(std::shared_ptr<Buffer> buf,
		BeeCallback callback) {
	std::unique_lock<std::mutex> lock(mutex_send_que_);
	send_que_.push(std::make_pair(buf, callback));
	lock.unlock();
	//std::unique_lock<std::mutex> l1(mutex_send_que_);
	if (mutex_send_que_.try_lock()) {
		//if ((send_que_.size() + 300 - 1) / 300 >= send_thread_cnt_) {
		//	service_.post(std::bind(&Service::SendBufferFronQue, this));
		//}
			if (send_thread_cnt_ < 4) 
				service_.post(std::bind(&Service::SendBufferFronQue, this));
		mutex_send_que_.unlock();
	}
}

void Service::SendBufferFronQue() {
	std::unique_lock<std::mutex> lock(mutex_send_que_);
	if (send_thread_cnt_ >= 2) return;
	++send_thread_cnt_;
	lock.unlock();
	while (true) {
		lock.lock();
		if (send_que_.empty()) {
			lock.unlock();
			break;
		}
		auto re = send_que_.front();
		send_que_.pop();
		lock.unlock();
		if (re.second)  {
			auto callback = re.second;
			sender_->SendBuffer(re.first, [callback, this](const Error error){
						service_.post(std::bind(callback, error));
					});	
		} else {
			sender_->SendBuffer(re.first);	
		}
	}
	lock.lock();
	--send_thread_cnt_;
	lock.unlock();
}

void Service::DoCallback() {
	while (true) {
		std::unique_lock<std::mutex> lock(mutex_arrived_package_);
		if (arrived_package_.empty()) break;
		auto pack = std::move(arrived_package_.front());
		arrived_package_.pop();
		lock.unlock();
		arrived_callback_->OnCallback(std::move(pack));
	}
}
