#ifndef SERVICE_H
#define SERVICE_H

#include "recover/RecoverManager.h"
#include "net/UDPSender.h"
#include "net/UDPReceiver.h"
#include "PackageControl.h"

#include <queue>

namespace Bee {
	class Service : public RecoverManager::Interface {
	public:
		//override RecoverManager::Interface
		void SendNack(const size_t package_num) override;
		void SendPackageTo(std::shared_ptr<Buffer> buf, const UDPEndPoint endpoint) override;

		static Service& GetService() {
			static Service instance;
			return instance;
		}

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
		~Service();
		void Init();

		void SetThreadCount(int thread_count);
		void SetPackageArrivedCallback(PackageControl::PackageArrivedCallback callback) {
			package_control_->SetPackageArrivedCallback(callback);
		}

		void SetLocalAddress(const std::string IP, const short port);
		void Run();
		void Stop();

		void SendPackage(std::unique_ptr<Package> package);
		void ReceivedHandler(std::unique_ptr<Buffer> buffer, UDPEndPoint endpoint);
		size_t GetRTT();
		void SetBufferOutTime(int ms); //ms
		void AddClient(const std::string IP, const short port);
		void RemoveClient(const std::string IP, const short port);
		void ConnectService(const std::string IP, const short port);
		void DeConnectService(const std::string IP, const short port);
		bool Request(std::unique_ptr<Package> package, UDPEndPoint endpoint = UDPEndPoint{"0.0.0.0", 0});
		void SetHeartbeatRate(const size_t rate) { sender_->SetHeartRate(rate); }
		//boost::asio::io_service& GetIOService() {
		//	return service_;
		//}

	private:
		void BufferNotFound();
		void OnDataRecived(std::unique_ptr<Buffer> buf);
		void OnNACKRecived(std::unique_ptr<Buffer> buf, UDPEndPoint endpoint);
		void OnNotFoundPackRecived(std::unique_ptr<Buffer> buf);
		//void OnHeartBeatReceived(const UDPEndPoint endpoint);
		void OnHeartBeatReceived(std::unique_ptr<Buffer> buf, const UDPEndPoint endpoint);
		void OnSYNCHeartBeatReceived(std::unique_ptr<Buffer> buf, const UDPEndPoint endpoint);
	};
}

#endif
