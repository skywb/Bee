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
		boost::asio::ip::udp::socket socket_;
		std::vector<std::thread> threads_;
		std::unique_ptr<PackageControl> package_control_;
		std::unique_ptr<RecoverManager> recover_manager_;
		std::unique_ptr<UDPReceiver> receiver_;
		std::unique_ptr<UDPSender> sender_;
		std::queue<Package> package_queue_;
	public:
		Service();
		// Only for Unittest
		Service(bool is_unittest, std::unique_ptr<PackageControl> package_control = nullptr, 
				std::unique_ptr<RecoverManager> recover_manager = nullptr,
				std::unique_ptr<UDPReceiver> receiver = nullptr,
				std::unique_ptr<UDPSender> sender = nullptr);
		~Service();
		void SetThreadCount(int thread_count);
		void SetPackageArrivedCallback(PackageControl::PackageArrivedCallback callback) {
			package_control_->SetPackageArrivedCallback(callback);
		}
		void SetLocalAddress(const std::string IP, const short port);
		void Run();
		void Stop();
		void SendPackage(std::unique_ptr<Package> package);
		void ReceivedHandler(std::unique_ptr<Buffer> buffer, UDPEndPoint endpoint);
		void GetRTT();
		void SetBufferOutTime(int ms); //ms
	private:
		void BufferNotFound();
		void OnDataRecived(std::unique_ptr<Buffer> buf);
		void OnNACKRecived(std::unique_ptr<Buffer> buf, UDPEndPoint endpoint);
		void OnNotFoundPackRecived(std::unique_ptr<Buffer> buf);
	};
}

#endif
