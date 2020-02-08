#ifndef SERVICE_H
#define SERVICE_H

#include "recover/RecoverManager.h"

namespace Bee {
	class Service : SenderDelgate, SenderDelgate {

	private:
		std::unique_ptr<RecoverManager> recover_manager_;
		std::unique_ptr<UDPReceiver> receiver_;
		std::unique_ptr<UDPSender> sender_;
		std::queue<Package> package_queue_;
		boost::asio::io_service service_;
		std::vector<std::thread> threads_;
		int attribute;
		std::unique_ptr<PackageControl> package_control_;

	public:
		void Init(int thread_count);

		void SendPackage(std::unique_ptr<Package> package);

		void ReceivedHandler(std::unique_ptr<Buffer> buffer);

		void GetRTT();
	};
}

#endif
