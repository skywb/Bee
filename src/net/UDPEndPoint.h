#ifndef UDPENDPOINT_H
#define UDPENDPOINT_H

#include <string>

namespace Bee {
	struct UDPEndPoint {
		std::string IP;
		short port;
		bool operator< (const UDPEndPoint& other) const  {
			if (IP == other.IP) return port < other.port;
			else return IP < other.IP;
		}
	};
}

#endif
