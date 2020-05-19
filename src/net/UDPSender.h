#ifndef UDPSENDER_H
#define UDPSENDER_H

#include "UDPEndPoint.h"
#include "package/Package.h"
#include "service/Error.h"

#include <map>
#include <queue>

#include <boost/asio.hpp>

namespace Bee {
	class UDPSender {
		private:
			struct TypeClient {
				TypeClient(UDPEndPoint endpoint) : 
					endpoint_(boost::asio::ip::address::from_string(endpoint.IP), endpoint.port) {
						time_ = std::chrono::system_clock::now();
						is_multicast_ = endpoint_.address().is_multicast();
					}
				boost::asio::ip::udp::endpoint endpoint_;
				std::chrono::time_point<std::chrono::system_clock> time_;
				bool is_multicast_ = false;
			};
			size_t heart_rate_;
			boost::asio::deadline_timer timer_clear_outtime_client_;
			std::map<UDPEndPoint, TypeClient> endpoints_;
			std::mutex mutex_endpoints_;
			boost::asio::ip::udp::socket& socket_;
			boost::asio::io_service& service_;
			typedef std::queue<std::pair<
				std::shared_ptr<Buffer>, BeeCallback>> Que_;
			std::shared_ptr<Que_> sending_que_;
			std::shared_ptr<Que_> wait_sending_que_;
			std::mutex mutex_send_que_;
			bool is_sending_ = false;

		public:
			UDPSender(boost::asio::ip::udp::socket& socket);
			~UDPSender();
			void SendBuffer(std::shared_ptr<Buffer> buf, BeeCallback callback = nullptr);
			void AsyncSendBuffer(std::shared_ptr<Buffer> buf, BeeCallback callback = nullptr);
			void SendBufferTo(std::shared_ptr<Buffer> buf, 
					const UDPEndPoint endpoint, std::function<void()> callback = nullptr);
			void SetHeartRate(unsigned int rate);
			size_t GetHeartRate() { return heart_rate_; }
			void AddClient(UDPEndPoint endpoint);
			void RemoveClient(UDPEndPoint endpoint);
			bool Heartbeat(const UDPEndPoint endpoint);
			size_t GetClientCount() {
				std::lock_guard<std::mutex> lock(mutex_endpoints_);
				return endpoints_.size();
			}
		private:
			void ClearOutTimeClient();
			void SendBufferFromQue();
	};
}

#endif
