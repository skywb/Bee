#ifndef RECOVERMANAGER_H
#define RECOVERMANAGER_H

#include "package/Package.h"

#include <map>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>

namespace Bee {
	class RecoverManager {

	private:
		std::map<size_t, std::shared_ptr<Buffer>> package_history_;
		std::mutex mutex_package_history_;
		boost::asio::io_service& service_;
		boost::asio::deadline_timer timer_pack_outtime_;

	public:
		RecoverManager(boost::asio::io_service& service);

		void PackageArrived(size_t package_num);

		void AddPackRecord(std::shared_ptr<Buffer> buf);

		std::shared_ptr<Buffer> NACKReceived(size_t pack_num);

	private:
		void AddPackToHistroy(size_t package_num, std::shared_ptr<Buffer> pack_data);

		void ClearOutTimeHistory();

	};
}

#endif
