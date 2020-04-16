#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include "package/Package.h"	
#include "net/UDPEndPoint.h"
#include <memory>
#include <list>
#include <functional>
#include <boost/asio.hpp>

namespace Bee {

	class MulcastReceiver
	{
	public:
		using TypeCallback = std::function<void(std::unique_ptr<Buffer>, UDPEndPoint)>;
	public:
		MulcastReceiver (boost::asio::io_service& service, const UDPEndPoint& endpoint, TypeCallback callback);
		virtual ~MulcastReceiver ();
		void Run();
		void Stop();
		const std::string MulcastIP() { return multicast_ip_; }
		const boost::asio::ip::udp::endpoint Endpoint() { return src_endpoint_; }
		bool IsAvailbale() { return available_; }
	private:
		void AsyncReceive ();
		boost::asio::ip::udp::socket socket_;
		const std::string multicast_ip_;
		const short multicast_port_;
		boost::asio::ip::udp::endpoint src_endpoint_;
		bool available_ = false;
		uint8_t* buf_;
		TypeCallback receive_callback_;
	};

	class AsyncReceiver
	{
	public:
		using TypeCallback = std::function<void(std::unique_ptr<Buffer>, UDPEndPoint)>;
	public:
		AsyncReceiver (boost::asio::ip::udp::socket& socket, TypeCallback callback) :
			socket_(socket), 
			receive_callback_(callback),
			buf_(nullptr) {
			running_ = false;
		}
		virtual ~AsyncReceiver () {
			running_ = false;
			if (buf_) {
			   	delete [] buf_;
				buf_ = nullptr;
			}
		}
		void Stop () { running_ = false; }
		void Run () { 
			if (buf_) delete [] buf_;
			buf_ = new uint8_t[1500];
		   	running_ = true;
			AsyncReceive();
	   	}
	private:
		void AsyncReceive ();
		boost::asio::ip::udp::socket& socket_;
		bool running_;
		uint8_t* buf_;
		boost::asio::ip::udp::endpoint remote_endpoint_;
		TypeCallback receive_callback_;
	};


	class UDPReceiver {

	public:
		using TypeCallback = std::function<void(std::unique_ptr<Buffer>, UDPEndPoint)>;

	private:
		boost::asio::ip::udp::socket& socket_;
		std::vector<std::unique_ptr<AsyncReceiver>> receivers_;
		std::map<UDPEndPoint, boost::asio::ip::udp::endpoint> services_;
		std::mutex mutex_servies_;
		std::list<std::unique_ptr<MulcastReceiver>> multicast_services_;
		std::mutex mutex_multicast_services_;
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
		void RemoveService(UDPEndPoint endpoint);

	private:
		bool IsServiceIP(const UDPEndPoint& ep);
		void AsyncHeartbeat();
		void SendHeartbeat();
	};


}

#endif
