#ifndef RECOVERMANAGER_H
#define RECOVERMANAGER_H

#include "package/Package.h"
#include "net/UDPEndPoint.h"

#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>
#include <atomic>

namespace Bee {
	class RecoverManager {

	public:
		class Interface
		{
		public:
			virtual void SendNack(const size_t package_num) = 0;
			virtual void SendPackageTo(std::shared_ptr<Buffer> buf, const UDPEndPoint endpoint) = 0;
			virtual void OnBufferOutTime(const size_t package_num) = 0;
		};
	private:
		std::map<size_t, std::shared_ptr<Buffer>> package_history_;
		std::mutex mutex_package_history_;
		std::unordered_map<size_t, 
			std::chrono::time_point<std::chrono::system_clock>> recover_wait_;
		std::mutex mutex_recover_wait_;
		boost::asio::io_service& service_;
		boost::asio::deadline_timer timer_NACK_tracer;
		std::atomic<size_t> rtt_;
		Interface* sender_;
		size_t min_pack_num_ = 0;
		size_t max_pack_num_ = 0;
		size_t oldest_nack_pack_num_ = 0;
		bool is_first_pack_num = true;
		int history_max_len_ = 10000;
	public:
		RecoverManager(boost::asio::io_service& service, Interface* sender);
		virtual ~RecoverManager();
		bool PackageArrived(size_t package_num);
		void AddPackRecord(std::shared_ptr<Buffer> buf);
		void NACKReceived(size_t pack_num, UDPEndPoint endpoint);
		size_t GetRTT() { return rtt_; }
		void SetHistoryMaxSize(const size_t size) { history_max_len_ = size; }
		bool WaitPackage(const size_t min_packnum, const size_t max_packnum);

	private:
		void AddPackToHistroy(size_t package_num, std::shared_ptr<Buffer> pack_data);
		std::shared_ptr<Buffer> GetHistory(size_t pack_num);
		void ClearOutTimeHistory();
		void NackTrackerHandler(const boost::system::error_code& error);
	};
}

#endif
