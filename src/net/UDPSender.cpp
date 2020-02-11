#include "UDPSender.h"
#include <boost/asio.hpp>

using namespace Bee;

UDPSender::UDPSender(boost::asio::ip::udp::socket& socket):
	socket_(socket) {
		heart_rate_ = 1000;
}

void UDPSender::SendBuffer(std::shared_ptr<Buffer> buf) {
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	for (auto i : endpoints_) {
		socket_.async_send_to(boost::asio::buffer(buf->GetBufferData(), buf->Size()), i.second.endpoint_,
				[buf](const boost::system::error_code& error, std::size_t size) {
					if (error) {
						//TODO: log message
					}
				});
	}
}

void UDPSender::SetHeartRate(unsigned int rate) {
	heart_rate_ = rate;
}

void UDPSender::AddClient(UDPEndPoint endpoint) {
	std::lock_guard<std::mutex> lock(mutex_endpoints_);
	boost::system::error_code ec;
	auto addr = boost::asio::ip::address::from_string(endpoint.IP, ec);
	if (ec) {
		//TODO: //log message
		return ;
	}
	auto it = endpoints_.emplace(std::make_pair (endpoint, endpoint));
	if (addr.is_multicast()) {
		it.first->second.is_multicast_ = true;
	}
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
		if (i->second.time_ + heart_rate_ < cur_time) {
			i = endpoints_.erase(i);
		} else {
			++i;
		}
	}
}
