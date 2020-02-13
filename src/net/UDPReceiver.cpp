#include "UDPReceiver.h"
#include "log/mlog.hpp"

using namespace Bee;

UDPReceiver::UDPReceiver(boost::asio::ip::udp::socket& socket, TypeCallback fun)
   	: socket_(socket),
	receive_callback_(fun),
	buf_(new uint8_t[1500]){ }

UDPReceiver::~UDPReceiver() {
	delete[]  buf_;
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

