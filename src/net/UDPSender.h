#ifndef UDPSENDER_H
#define UDPSENDER_H

#include "UDPEndPoint.h"
#include "package/Package.h"

#include <map>

#include <boost/asio.hpp>

namespace Bee {
	class UDPSender {

	private:
		struct type_client {
			type_client(UDPEndPoint endpoint) : 
				endpoint_(boost::asio::ip::address::from_string(endpoint.IP), endpoint.port) {
					time_ = std::chrono::system_clock::now();
			}
			boost::asio::ip::udp::endpoint endpoint_;
			std::chrono::time_point<std::chrono::system_clock> time_;
			bool is_multicast_ = false;
		};
		std::chrono::milliseconds heart_rate_;
		std::map<UDPEndPoint, type_client> endpoints_;
		std::mutex mutex_endpoints_;
		boost::asio::ip::udp::socket& socket_;

	public:
		UDPSender(boost::asio::ip::udp::socket& socket);
		~UDPSender();

		void SendBuffer(std::shared_ptr<Buffer> buf);
		void SendBufferTo(std::shared_ptr<Buffer> buf, UDPEndPoint endpoint);

		void SetHeartRate(unsigned int rate);
		void AddClient(UDPEndPoint endpoint);
		void RemoveClient(UDPEndPoint endpoint);
		bool Heartbeat(const UDPEndPoint endpoint);

	private:
		void ClearOutTimeClient();
	};
}

#endif
