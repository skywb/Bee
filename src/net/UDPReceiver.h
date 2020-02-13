#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include "package/Package.h"	
#include "net/UDPEndPoint.h"
#include <memory>
#include <functional>
#include <boost/asio.hpp>

namespace Bee {
	class UDPReceiver {

	public:
		using TypeCallback = std::function<void(std::unique_ptr<Buffer>, UDPEndPoint)>;

	private:
		boost::asio::ip::udp::socket& socket_;
		boost::asio::ip::udp::endpoint remote_endpoint_;
		TypeCallback receive_callback_;
		uint8_t* buf_;

	public:
		UDPReceiver(boost::asio::ip::udp::socket& socket, TypeCallback fun);
		virtual ~UDPReceiver();
		void AsyncReceive();
	};
}

#endif
