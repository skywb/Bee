#ifndef UDPSENDER_H
#define UDPSENDER_H

#include "UDPEndPoint.h"

#include <map>

#include <boost/asio.hpp>

namespace Bee {
	class UDPSender {

	private:
		std::map<UDPEndPoint, boost::asio::ip::udp::endpoint> endpoints_;
		boost::asio::ip::udp::socket& socket_;
		int heart_rate_;

	public:
		void SetHeartRate();

		void AddClient(int UDPEndPoint_endpoint);

		void RemoveClient(int UDPEndPoint_endpoint);

	private:
		void ClearOutTimeClient();
	};
}

#endif
