#include "UDPSender.h"
#include "log/mlog.hpp"
#include <boost/asio.hpp>

using namespace Bee;

UDPSender::UDPSender(boost::asio::ip::udp::socket& socket):
	socket_(socket),
	timer_heartbeat_(socket.get_io_service()) {
		heart_rate_ = std::chrono::milliseconds(1000);
 }

UDPSender::~UDPSender() { }

void UDPSender::SendBuffer(std::shared_ptr<Buffer> buf, std::function<void()> callback) {
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	bool is_send = false;
	for (auto i : endpoints_) {
		is_send = true;
		socket_.async_send_to(boost::asio::buffer(buf->GetBufferData(), 
				buf->GetBufferSize()), i.second.endpoint_,
				[buf, callback](const boost::system::error_code& error, std::size_t size) {
					auto foo = buf;
					if (error) {
						LOG_ERROR << "Send error : " << error.message();
						return;
					}
					callback();
				});
	}
	if (!is_send && callback) {
		callback();
	}
}

void UDPSender::SendBufferTo(std::shared_ptr<Buffer> buf, 
		const UDPEndPoint endpoint, std::function<void()> callback) {
	boost::asio::ip::udp::endpoint ep(boost::asio::ip::address::from_string(endpoint.IP), endpoint.port);
	socket_.async_send_to(boost::asio::buffer(buf->GetBufferData(), buf->GetBufferSize()), ep, 
			[=](const boost::system::error_code& error, size_t size) {
				auto tmp = buf;	
				if (error) {
					LOG_ERROR << "Send error : " << error.message();
					LOG_ERROR << buf->GetBufferHeader().pack_num;
				}
				callback();
			});
}

void UDPSender::SetHeartRate(unsigned int rate) {
	heart_rate_ = std::chrono::milliseconds(rate);
}

void UDPSender::AddClient(UDPEndPoint endpoint) {
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	boost::system::error_code ec;
	auto addr = boost::asio::ip::address::from_string(endpoint.IP, ec);
	if (ec) {
		LOG_ERROR << "Add endpoint error : " << ec.message();
		return ;
	}
	auto it = endpoints_.emplace(std::make_pair (endpoint, endpoint));
}

void UDPSender::RemoveClient(UDPEndPoint endpoint) {
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	auto it = endpoints_.find(endpoint);
	if (it != endpoints_.end()) 
		endpoints_.erase(it);
}

void UDPSender::ClearOutTimeClient() {
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	auto cur_time = std::chrono::system_clock::now();
	for (auto i = endpoints_.begin(); i != endpoints_.end();) {
		if (i->second.is_multicast_) {
			++i;
			continue;
		} 
		if (i->second.time_ + heart_rate_*2 < cur_time) {
			i = endpoints_.erase(i);
		} else {
			++i;
		}
	}
}

bool UDPSender::Heartbeat(const UDPEndPoint endpoint) {
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	auto it = endpoints_.find(endpoint);
	if (it == endpoints_.end()) 
		return false;
	else {
		it->second.time_ = std::chrono::system_clock::now();
		return true;
	}
}
