#ifndef UDPENDPOINT_H
#define UDPENDPOINT_H

#include <string>

namespace Bee {
	struct UDPEndPoint {
		UDPEndPoint (const std::string IP, const short port) {
			this->IP = IP;
			this->port = port;
			hash_val_ = Hash(IP, port);
		}
		UDPEndPoint(const UDPEndPoint& foo) {
			IP = foo.IP;
			port = foo.port;
			hash_val_ = Hash(IP, port);
		}
		UDPEndPoint() = default;
		~UDPEndPoint() = default;
		std::string IP;
		short port;
		long long hash_val_;
		bool operator< (const UDPEndPoint& other) const  {
			return this->hash_val_ < other.hash_val_;
		}
		bool operator== (const UDPEndPoint& other) const  {
			return this->hash_val_ == other.hash_val_;
		}
		UDPEndPoint& operator= (const UDPEndPoint& other) {
			IP = other.IP;
			port = other.port;
			hash_val_ = other.hash_val_;
			return *this; 
		}
		long long Hash(const std::string& IP, const short& port) {
			long long res = 0;
			long long cur = 0;
			for (int i = 0; i < IP.size(); ++i) {
				if (IP[i] == '.') {
					res *= 255;
					res += cur;
					cur = 0;
				} else {
					cur *= 10;
					cur += IP[i] - '0';
				}
			}
			res *= 255;
			res += cur;
			res *= 100000;
			res += port;
			return res;
		}
	};
}

#endif
