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
		std::map<UDPEndPoint, boost::asio::ip::udp::endpoint> services_;
		std::mutex mutex_servies_;
		boost::asio::deadline_timer timer_heart_;
		size_t heartbeat_rate_ = 100; //ms
		TypeCallback receive_callback_;
		uint8_t* buf_;
		Buffer buf_heartbeat_;

	public:
		UDPReceiver(boost::asio::ip::udp::socket& socket, TypeCallback callback);
		virtual ~UDPReceiver();
		void AsyncReceive();
		void SendBufferToService(std::unique_ptr<Buffer> buf, const bool allService = false);
		void SetHeartbeatRate(size_t rate /*ms*/) { heartbeat_rate_ = rate; }
		void AddService(UDPEndPoint endpoint);

	private:
		void AsyncHeartbeat();
		void SendHeartbeat();
	};
}

#endif
