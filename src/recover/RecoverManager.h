#ifndef RECOVERMANAGER_H
#define RECOVERMANAGER_H

#include "package/Package.h"
#include "net/UDPEndPoint.h"

#include <map>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>

namespace Bee {
	class RecoverManager {

	public:
		typedef std::function<void(std::shared_ptr<Buffer>, UDPEndPoint)> SenderCallback;

	private:
		std::map<size_t, std::shared_ptr<Buffer>> package_history_;
		std::mutex mutex_package_history_;
		std::map<size_t, std::chrono::time_point<std::chrono::system_clock>> recover_wait_;
		std::mutex mutex_recover_wait_;
		boost::asio::io_service& service_;
		boost::asio::deadline_timer timer_pack_outtime_;

		SenderCallback sender_;

		std::mutex mutex_pack_num_;
		size_t min_pack_num_ = 0;
		size_t max_pack_num_ = 0;
		size_t oldest_nack_pack_num_ = 0;
		bool is_first_pack_num = true;

	public:
		RecoverManager(boost::asio::io_service& service, SenderCallback sender);
		virtual ~RecoverManager();

		void PackageArrived(size_t package_num);

		void AddPackRecord(std::shared_ptr<Buffer> buf);

		void NACKReceived(size_t pack_num, UDPEndPoint endpoint);

	private:
		void AddPackToHistroy(size_t package_num, std::shared_ptr<Buffer> pack_data);
		std::shared_ptr<Buffer> GetHistory(size_t pack_num);

		void ClearOutTimeHistory();

	};
}

#endif
