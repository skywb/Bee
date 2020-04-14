#ifndef UDPENDPOINT_H
#define UDPENDPOINT_H

#include <string>

namespace Bee {
	struct UDPEndPoint {
		UDPEndPoint (const std::string IP, const short port) {
			this->IP = IP;
			this->port = port;
		}
		UDPEndPoint(const UDPEndPoint& foo) {
			IP = foo.IP;
			port = foo.port;
		}
		UDPEndPoint() = default;
		~UDPEndPoint() = default;
		std::string IP;
		short port;
		bool operator< (const UDPEndPoint& other) const  {
			if (IP == other.IP) return port < other.port;
			else return IP < other.IP;
		}
		UDPEndPoint& operator= (const UDPEndPoint& other) {
			IP = other.IP;
			port = other.port;
			return *this; }
	};
}

#endif
