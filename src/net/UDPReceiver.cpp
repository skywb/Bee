#include "UDPReceiver.h"
#include "log/mlog.hpp"

#include <memory>

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
					LOG_INFO << "Send heartbeat " ;
				});
	}
}

void UDPReceiver::AsyncReceive() {
	AddOneReceiver();
	LOG_DEBUG << "Add a Receiver";
	for (auto& i : multicast_services_) {
		i->Run();
	}
}
void UDPReceiver::AddOneReceiver() {
	std::lock_guard<std::mutex> lock(mutex_receivers_);
	receivers_.emplace_back(std::make_unique<AsyncReceiver>(socket_,
				[&](std::unique_ptr<Buffer> buf, UDPEndPoint endpoint){
					receive_callback_(std::move(buf), endpoint);
				}));
	receivers_[receivers_.size()-1]->Run();
}

bool UDPReceiver::IsServiceIP(const UDPEndPoint& ep) {
	std::lock_guard<std::mutex> lock(mutex_servies_);
	auto it = services_.find(ep);
	if (it == services_.end()) return false;
	return true;
}

void UDPReceiver::SendBufferToService(std::unique_ptr<Buffer> buf, const bool allService) {
	std::unique_lock<std::mutex> lock(mutex_servies_);
	for (auto i : services_) {
		socket_.async_send_to(boost::asio::buffer(buf->GetBufferData(), buf->GetBufferSize()),
			   	i.second,
			   	[](const boost::system::error_code& error, std::size_t size){});
	}
	lock.unlock();
	std::unique_lock<std::mutex> mul_lock(mutex_multicast_services_);
	for (auto& i : multicast_services_) {
		if (i->IsAvailbale()) {
		socket_.async_send_to(boost::asio::buffer(buf->GetBufferData(), buf->GetBufferSize()),
			   	i->Endpoint(),
			   	[](const boost::system::error_code& error, std::size_t size){});
		}
	}
}

void UDPReceiver::AddService(UDPEndPoint endpoint) {
	auto addr = boost::asio::ip::address::from_string(endpoint.IP);
	if (addr.is_multicast()) {
		std::lock_guard<std::mutex> lock(mutex_multicast_services_);
		auto mul_serveice = std::make_unique<MulcastReceiver> (
				socket_.get_io_service(), endpoint, receive_callback_);
		mul_serveice->Run();
		multicast_services_.push_back(std::move(mul_serveice));
	} else {
		std::unique_lock<std::mutex> lock(mutex_servies_);
		auto it = services_.find(endpoint);
		if (it == services_.end()) {
			services_.emplace(endpoint, boost::asio::ip::udp::endpoint(
						boost::asio::ip::address::from_string(endpoint.IP),
					   	endpoint.port));
			LOG_INFO << "Add a service : " << endpoint.IP << ":"
			   	<< endpoint.port << " current service count is " << services_.size();
		}
		lock.unlock();
		SendHeartbeat();
	}
}

void UDPReceiver::RemoveService(UDPEndPoint endpoint) {
	auto addr = boost::asio::ip::address::from_string(endpoint.IP);
	if (addr.is_multicast()) {
		std::lock_guard<std::mutex> lock(mutex_multicast_services_);
		for (auto it = multicast_services_.begin(); it != multicast_services_.end(); ++it) {
			if ((*it)->MulcastIP() == endpoint.IP) {
				multicast_services_.erase(it);
			}
		}
	} else {
		std::unique_lock<std::mutex> lock(mutex_servies_);
		auto it = services_.find(endpoint);
		if (it != services_.end()) {
			services_.erase(endpoint);
		}
		lock.unlock();
	}
}

void AsyncReceiver::AsyncReceive() {
	socket_.async_receive_from(boost::asio::buffer(buf_, 1500), remote_endpoint_, 
			[&](const boost::system::error_code& error, std::size_t size) {
				if (error) {
					if (error.value() != 10061) {
						LOG_WARN << "receive error " << error <<  ": " << error.message();
					}
					AsyncReceive();
					return ;
				}
				UDPEndPoint endpoint;
				endpoint.IP = remote_endpoint_.address().to_string();
				endpoint.port = remote_endpoint_.port();
				auto buf = std::make_unique<Buffer> (buf_, size);
				receive_callback_(std::move(buf), endpoint);
				AsyncReceive();
			});
}

MulcastReceiver::MulcastReceiver (boost::asio::io_service& service,
	   	const UDPEndPoint& endpoint, TypeCallback callback) :
#ifdef LINUX
	socket_(service, boost::asio::ip::udp::endpoint(
				boost::asio::ip::address::from_string(endpoint.IP), endpoint.port))
#elif WIN
	socket_(service, boost::asio::ip::udp::endpoint(
				boost::asio::ip::address::from_string("0.0.0.0"), endpoint.port))
#else
	socket_(service, boost::asio::ip::udp::endpoint(
				boost::asio::ip::address::from_string("0.0.0.0"), endpoint.port))
#endif
	,multicast_ip_(endpoint.IP),
	multicast_port_(endpoint.port),
	buf_(nullptr), receive_callback_(callback)
{
	LOG_INFO << "multicast is " << endpoint.IP;
	boost::system::error_code error;
	socket_.set_option(boost::asio::ip::multicast::join_group(
				boost::asio::ip::address::from_string(multicast_ip_)), error);
	if (error) {
		LOG_INFO << error.message();
	}
}

MulcastReceiver::~MulcastReceiver () {
	socket_.set_option(boost::asio::ip::multicast::leave_group(
				boost::asio::ip::address::from_string(multicast_ip_)));
	if (socket_.is_open())
		socket_.close();
}

void MulcastReceiver::Run() {
	if (buf_) delete [] buf_;
	buf_ = new uint8_t[1500];
	AsyncReceive();
}

void MulcastReceiver::AsyncReceive() {
	socket_.async_receive_from(boost::asio::buffer(buf_, 1500), src_endpoint_, 
			[&](const boost::system::error_code& error, std::size_t size) {
				if (error) {
					LOG_WARN << "receive error : " << error.message();
					AsyncReceive();
					return ;
				}
				available_ = true;
				UDPEndPoint endpoint;
				endpoint.IP = src_endpoint_.address().to_string();
				endpoint.port = src_endpoint_.port();
				auto buf = std::make_unique<Buffer> (buf_, size);
				receive_callback_(std::move(buf), endpoint);
				AsyncReceive();
			});
}

