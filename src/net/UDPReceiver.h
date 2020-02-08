#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include "package/Package.h"
#include <memory>
#include <functional>
#include <boost/asio.hpp>

namespace Bee {
	class UDPReceiver {

	private:
		boost::asio::ip::udp::socket& socket_;
		boost::asio::ip::udp::endpoint remote_endpoint_;
		std::function<void(std::unique_ptr<Buffer>)> receive_callback_;
		uint8_t* buf_;

	public:
		using type_callback = std::function<void(std::unique_ptr<Buffer>)>;
		UDPReceiver(boost::asio::ip::udp::socket& socket, type_callback fun);
		~UDPReceiver();
		void AsyncReceive();
	};
}

#endif
