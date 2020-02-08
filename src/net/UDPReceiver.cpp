#include "UDPReceiver.h"

using namespace Bee;

UDPReceiver::UDPReceiver(boost::asio::ip::udp::socket& socket, type_callback fun)
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
					AsyncReceive();
					//TODO: LOG_MSG
				}
				auto buf = std::make_unique<Buffer> (buf_, size);
				receive_callback_(std::move(buf));
				AsyncReceive();
			});
}

