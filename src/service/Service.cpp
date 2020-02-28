#include "Service.h"
#include "service/mlog.h"

#include <random>

using namespace Bee;

void Service::SendNack(const size_t package_num) {
	LOG_DEBUG << "Send NACK";
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
	socket_(service_) {
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

//// Only for Unittest
//Service::Service(bool is_unittest, std::unique_ptr<PackageControl> package_control, 
//		std::unique_ptr<RecoverManager> recover_manager,
//		std::unique_ptr<UDPReceiver> receiver,
//		std::unique_ptr<UDPSender> sender) : socket_(service_) {
//	assert(is_unittest);
//	if (package_control)
//		package_control_ = std::move(package_control);
//	else 
//		package_control_ = std::make_unique<PackageControl> ();
//	if (recover_manager)
//		recover_manager_ = std::move(recover_manager);
//	else 
//		recover_manager_ = std::make_unique<RecoverManager> (service_, [&](std::shared_ptr<Buffer> buf, UDPEndPoint endpoint) {
//				sender_->SendBufferTo(buf, endpoint);
//				});
//	if (receiver)
//		receiver_ = std::move(receiver);
//	else 
//		receiver_ = std::make_unique<UDPReceiver> (socket_,
//				std::bind(&Service::ReceivedHandler, this, std::placeholders::_1, std::placeholders::_2));
//	if (sender)
//		sender_ = std::move(sender);
//	else 
//		sender_ = std::make_unique<UDPSender> (socket_);
//}

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
	socket_.bind(local_endpoint, error);
	if (error) {
		LOG_WARN << "bind error : " << error.message();
	}
}

void Service::Run() {
	receiver_->AsyncReceive();
}

void Service::Stop() {
	service_.stop();
}

void Service::SendPackage(std::unique_ptr<Package> package) {
	auto buffers = std::move(package_control_->SplitPackage(std::move(package)));
	for (int i=0; i<buffers.size(); ++i) {
		std::shared_ptr<Buffer> buffer(buffers[i].release());
		sender_->SendBuffer(buffer);
		recover_manager_->AddPackRecord(buffer);
	}
}


void Service::ReceivedHandler(std::unique_ptr<Buffer> buffer, UDPEndPoint endpoint) {
	std::random_device rd;
	std::uniform_int_distribution<> dis(0,9);
	switch (buffer->GetBufferHeader().type) {
		case BufferType::DATA:
			if (dis(rd) < 2) {
				break;
			}
			OnDataRecived(std::move(buffer));
			break;
		case BufferType::NACK:
			LOG_INFO << "NACK";
			OnNACKRecived(std::move(buffer), endpoint);
			break;
		case BufferType::BUFFER_NOT_FOUND:
			LOG_INFO << "BUFFER_NOT_FOUND";
			OnNotFoundPackRecived(std::move(buffer));
			break;
		case BufferType::HEARTBEAT:
			//OnNotFoundPackRecived(std::move(buffer));
			OnHeartBeatReceived(std::move(buffer), endpoint);
			break;
		case BufferType::SYNC_HEATBEAT:
			OnSYNCHeartBeatReceived(std::move(buffer), endpoint);
			break;
		default:
			LOG_ERROR << "not found this type";
	}
}

size_t Service::GetRTT() {
	return recover_manager_->GetRTT();
}

void Service::OnDataRecived(std::unique_ptr<Buffer> buf) {
	recover_manager_->PackageArrived(buf->GetBufferHeader().pack_num);
	package_control_->OnReceivedBuffer(std::move(buf));
}

void Service::OnNACKRecived(std::unique_ptr<Buffer> buf, UDPEndPoint endpoint) {
	recover_manager_->NACKReceived(buf->GetBufferHeader().pack_num, endpoint);
}

void Service::OnNotFoundPackRecived(std::unique_ptr<Buffer> buf) {
	package_control_->OnBufferNotFound(buf->GetBufferHeader().pack_num);
}

void Service::OnHeartBeatReceived(std::unique_ptr<Buffer> buf, const UDPEndPoint endpoint) {
	if (!sender_->Heartbeat(endpoint)) {
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
	receiver_->SendBufferToService(std::move(buf));
	return true;
}
