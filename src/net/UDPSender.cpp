#include "UDPSender.h"
#include "log/mlog.hpp"
#include <boost/asio.hpp>

using namespace Bee;

UDPSender::UDPSender(boost::asio::ip::udp::socket& socket):
	socket_(socket),
	service_(socket.get_io_service()),
	timer_clear_outtime_client_(socket.get_io_service()) {
		sending_que_ = std::make_shared<Que_>();
		wait_sending_que_ = std::make_shared<Que_>();
		heart_rate_ = 1000;
		timer_clear_outtime_client_.expires_from_now(boost::posix_time::milliseconds(heart_rate_*3));
		timer_clear_outtime_client_.async_wait(std::bind(&UDPSender::ClearOutTimeClient, this));
 }

UDPSender::~UDPSender() { }

void UDPSender::SendBuffer(std::shared_ptr<Buffer> buf, BeeCallback callback) {
	std::unique_lock<std::mutex> lock(mutex_endpoints_);
	bool is_send = false;
	for (auto i : endpoints_) {
		is_send = true;
		socket_.async_send_to(boost::asio::buffer(buf->GetBufferData(), 
				buf->GetBufferSize()), i.second.endpoint_,
				[buf, callback, ip = i.second.endpoint_.address()]
				(const boost::system::error_code& error, std::size_t size) {
					auto foo = buf;
					if (error) {
						LOG_ERROR << "Send error : " << error.message() << "  IP: " << ip;
						return;
					}
					if (callback) {
						callback(Error::kSusseed);
					}
				});
	}
	lock.unlock();
	if (!is_send && callback) {
		callback(Error(Error::kClientEmpty));
	}
}

void UDPSender::AsyncSendBuffer(std::shared_ptr<Buffer> buf, BeeCallback callback) {
	std::unique_lock<std::mutex> lock(mutex_send_que_);
	wait_sending_que_->push(std::make_pair(buf, callback));
	lock.unlock();
	if (mutex_send_que_.try_lock()) {
		if (!is_sending_)
			service_.post(std::bind(&UDPSender::SendBufferFromQue, this));
		mutex_send_que_.unlock();
	}
}

void UDPSender::SendBufferFromQue() {
	std::unique_lock<std::mutex> lock(mutex_send_que_);
	if (is_sending_) {
		lock.unlock();
		return;
	}
	is_sending_ =true;
	lock.unlock();
	while (true) {
		if (sending_que_->empty()) {
			lock.lock();
			if (!wait_sending_que_->empty()) {
				std::swap(wait_sending_que_, sending_que_);
			}
			lock.unlock();
			if (sending_que_->empty()) break;
		}
		auto re = sending_que_->front();
		sending_que_->pop();
		if (re.second)  {
			auto callback = re.second;
			SendBuffer(re.first, [callback, this](const Error error){
						service_.post(std::bind(callback, error));
					});	
		} else {
			SendBuffer(re.first);	
		}
	}
	lock.lock();
	is_sending_ = false;
	lock.unlock();
}

void UDPSender::SendBufferTo(std::shared_ptr<Buffer> buf, 
		const UDPEndPoint endpoint, std::function<void()> callback) {
	boost::asio::ip::udp::endpoint ep(
			boost::asio::ip::address::from_string(endpoint.IP), endpoint.port);
	socket_.async_send_to(
			boost::asio::buffer(buf->GetBufferData(),
			   	buf->GetBufferSize()), ep, 
			[buf, callback](const boost::system::error_code& error, size_t size) {
				auto tmp = buf;	
				if (error) {
					LOG_ERROR << "Send error : " << error.message();
					LOG_ERROR << buf->GetBufferHeader().pack_num;
				}
				if (callback) {
					callback();
				}
			});
}

void UDPSender::SetHeartRate(unsigned int rate) {
	heart_rate_ = rate;
}

void UDPSender::AddClient(UDPEndPoint endpoint) {
	LOG_INFO;
	boost::system::error_code ec;
	auto addr = boost::asio::ip::address::from_string(endpoint.IP, ec);
	if (ec) {
		LOG_ERROR << "Add endpoint error : " << ec.message();
		return ;
	}
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	//auto it = endpoints_.emplace(std::make_pair (endpoint, endpoint));
	auto it = endpoints_.emplace(endpoint, endpoint);
}

void UDPSender::RemoveClient(UDPEndPoint endpoint) {
	LOG_INFO;
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	auto it = endpoints_.find(endpoint);
	if (it != endpoints_.end()) 
		endpoints_.erase(it);
}

void UDPSender::ClearOutTimeClient() {
	LOG_INFO;
	auto cur_time = std::chrono::system_clock::now();
	std::unique_lock<std::mutex> lock(mutex_endpoints_);
	for (auto i = endpoints_.begin(); i != endpoints_.end();) {
		if (i->second.is_multicast_) {
			++i;
			continue;
		} 
		if (i->second.time_ + std::chrono::milliseconds(heart_rate_*4) < cur_time) {
			LOG_INFO << "remove client " << i->first.IP << " : " << i->first.port;
			i = endpoints_.erase(i);
		} else {
			++i;
		}
	}
	lock.unlock();
	timer_clear_outtime_client_.expires_from_now(boost::posix_time::milliseconds(heart_rate_*3));
	timer_clear_outtime_client_.async_wait(std::bind(&UDPSender::ClearOutTimeClient, this));
}

bool UDPSender::Heartbeat(const UDPEndPoint endpoint) {
	LOG_INFO;
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	auto it = endpoints_.find(endpoint);
	if (it == endpoints_.end()) 
		return false;
	else {
		auto now = std::chrono::system_clock::now();
		it->second.time_ = now;
		return true;
	}
}

