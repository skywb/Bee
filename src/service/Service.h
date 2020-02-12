#ifndef SERVICE_H
#define SERVICE_H

#include "recover/RecoverManager.h"
#include "net/UDPSender.h"
#include "net/UDPReceiver.h"
#include "PackageControl.h"

#include <queue>

namespace Bee {
	class Service {

	private:
		boost::asio::io_service service_;
		std::vector<std::thread> threads_;

		std::unique_ptr<PackageControl> package_control_;
		std::unique_ptr<RecoverManager> recover_manager_;
		std::unique_ptr<UDPReceiver> receiver_;
		std::unique_ptr<UDPSender> sender_;
		std::queue<Package> package_queue_;
	public:


		Service();
		// Only for Unittest
		Service(std::unique_ptr<PackageControl> package_control = nullptr, 
				std::unique_ptr<RecoverManager> recover_manager = nullptr,
				std::unique_ptr<UDPReceiver> receiver = nullptr,
				std::unique_ptr<UDPSender> sender = nullptr);


		void Init(int thread_count);

		void SetPackageArrivedCallback(PackageControl::PackageArrivedCallback callback) {
			package_control_->SetPackageArrivedCallback(callback);
		}

		void SendPackage(std::unique_ptr<Package> package);

		void ReceivedHandler(std::unique_ptr<Buffer> buffer);

		void GetRTT();

		
	
	private:
		void BufferNotFound();
	};
}

#endif
