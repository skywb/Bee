#include "UDPReceiver.h"
#include "log/mlog.hpp"

using namespace Bee;

UDPReceiver::UDPReceiver(boost::asio::ip::udp::socket& socket, TypeCallback callback)
   	: socket_(socket),
	receive_callback_(callback),
	timer_heart_(socket.get_io_service()),
	buf_(new uint8_t[1500]){
		AsyncHeartbeat();
		buf_heartbeat_.SetBufferType(BufferType::HEARTBEAT);
		buf_heartbeat_.SetData(nullptr, 0);
   	}


UDPReceiver::~UDPReceiver() {
	delete[]  buf_;
}

void UDPReceiver::AsyncHeartbeat() {
	SendHeartbeat();
	timer_heart_.expires_from_now(boost::posix_time::milliseconds(heartbeat_rate_));
	timer_heart_.async_wait([&](const boost::system::error_code& error){
				if (error) {
					return;
				}
				AsyncHeartbeat();
			});
}

void UDPReceiver::SendHeartbeat() {
	std::lock_guard<std::mutex> lock(mutex_servies_);
	buf_heartbeat_.SetHeartRate(heartbeat_rate_);
	for (auto i : services_) {
		socket_.async_send_to(boost::asio::buffer(buf_heartbeat_.GetBufferData(), buf_heartbeat_.GetBufferSize()),
			   	i.second,
			   	[](const boost::system::error_code& error, std::size_t size){
					if (error) {
						LOG_ERROR << error.message();
					}
				});
	}

}

void UDPReceiver::AsyncReceive() {
	socket_.async_receive_from(boost::asio::buffer(buf_, 1500), remote_endpoint_, 
			[&](const boost::system::error_code& error, std::size_t size) {
				if (error) {
					LOG_WARN << "receive error : " << error.message();
					AsyncReceive();
				}
				auto buf = std::make_unique<Buffer> (buf_, size);
				UDPEndPoint endpoint;
				endpoint.IP = remote_endpoint_.address().to_string();
				endpoint.port = remote_endpoint_.port();
				receive_callback_(std::move(buf), endpoint);
				AsyncReceive();
			});
}

void UDPReceiver::SendBufferToService(std::unique_ptr<Buffer> buf, const bool allService) {
	socket_.async_send_to(boost::asio::buffer(buf->GetBufferData(), buf->GetBufferSize()),
		   	remote_endpoint_,
		   	[](const boost::system::error_code& error, std::size_t size){});
}

void UDPReceiver::AddService(UDPEndPoint endpoint) {
	std::unique_lock<std::mutex> lock(mutex_servies_);
	auto it = services_.find(endpoint);
	if (it == services_.end()) {
		services_.emplace(endpoint, boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(endpoint.IP), endpoint.port));
		LOG_INFO << "Add a service : " << endpoint.IP << ":" << endpoint.port << " current service count is " << services_.size();
	}
	lock.unlock();
	SendHeartbeat();
}

void UDPReceiver::RemoveService(UDPEndPoint endpoint) {
	std::unique_lock<std::mutex> lock(mutex_servies_);
	auto it = services_.find(endpoint);
	if (it != services_.end()) {
		services_.erase(endpoint);
	}
	lock.unlock();
}
