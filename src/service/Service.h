#ifndef SERVICE_H
#define SERVICE_H

#include "recover/RecoverManager.h"
#include "net/UDPSender.h"
#include "net/UDPReceiver.h"
#include "PackageControl.h"
#include "log/mlog.hpp"

#include <queue>
#include <utility>

namespace Bee {
	class PackageArrivedCallback {
		public:
			virtual void OnCallback(std::unique_ptr<Bee::Package> package) = 0;
	};

	class PackageSendedCallback {
		public:
			virtual void OnCallback(std::unique_ptr<Bee::Package> package) = 0;
	};

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
		bool transpond_ = false;
		// unprocessed buffer
		std::queue<std::unique_ptr<Buffer>> buffer_que_;
		std::mutex mutex_buffer_que_;
		int using_process_thread_cnt_ = 0;
		std::mutex mutex_using_process_thread_cnt_;
		std::unique_ptr<PackageArrivedCallback> arrived_callback_;
		std::unique_ptr<PackageSendedCallback> sended_callback_;
		//send buffer que
		std::queue<std::pair<
			std::shared_ptr<Buffer>,
		   	std::function<void(void)>>> send_que_;
		std::mutex mutex_send_que_;
		int send_thread_cnt_ = 0;

	public:
		Service();
		~Service();
		void Init();

		void SetThreadCount(int thread_count);
		void SetPackageArrivedCallback(std::unique_ptr<PackageArrivedCallback> callback) {
			arrived_callback_ = std::move(callback);
			package_control_->SetPackageArrivedCallback([&](std::unique_ptr<Package> pack){
					if (arrived_callback_)
						arrived_callback_->OnCallback(std::move(pack));
					else 
						LOG_ERROR << "arrived_callback_ is null";
				});
		}

		void SetLocalAddress(const std::string IP, const short port);
		void Run();
		void Stop();
		void SendPackage(std::unique_ptr<Package> package, std::function<void(void)> callback = [](){});
		void ReceivedHandler(std::unique_ptr<Buffer> buffer, UDPEndPoint endpoint);
		size_t GetRTT();
		void SetBufferOutTime(int ms); //ms
		void SetBufferMaxCount(const size_t size); //ms
		void AddClient(const std::string IP, const short port);
		void RemoveClient(const std::string IP, const short port);
		void ConnectService(const std::string IP, const short port);
		void DeConnectService(const std::string IP, const short port);
		bool Request(std::unique_ptr<Package> package, UDPEndPoint endpoint = UDPEndPoint{"0.0.0.0", 0});
		void SetHeartbeatRate(const size_t rate) { sender_->SetHeartRate(rate); }
		void SetTranspond(bool transpond);

	private:
		void BufferNotFound();
		void OnDataRecived(std::unique_ptr<Buffer> buf);
		void OnNACKRecived(std::unique_ptr<Buffer> buf, UDPEndPoint endpoint);
		void OnNotFoundPackRecived(std::unique_ptr<Buffer> buf);
		//void OnHeartBeatReceived(const UDPEndPoint endpoint);
		void OnHeartBeatReceived(std::unique_ptr<Buffer> buf, const UDPEndPoint endpoint);
		void OnSYNCHeartBeatReceived(std::unique_ptr<Buffer> buf, const UDPEndPoint endpoint);
		void AddBufferToQue(std::unique_ptr<Buffer> buf);
		std::unique_ptr<Buffer> GetBufferFromQue();
		void AsyncProcessBuffers();
		void DoSendPackage(std::shared_ptr<Package> package, std::function<void(void)> callback);
		void AddBufferToSendQue(std::shared_ptr<Buffer> buf,
			   	std::function<void(void)> callback = [](){});
		void SendBufferFronQue();
	};

	class PackageArrivedCallbackDefault : public PackageArrivedCallback {
		public:
			PackageArrivedCallbackDefault()  = default;
			~PackageArrivedCallbackDefault()  = default;
			virtual void OnCallback(std::unique_ptr<Bee::Package> package) {}
	};

	class PackageSendedCallbackDefault : public PackageSendedCallback {
		public:
			PackageSendedCallbackDefault()  = default;
			~PackageSendedCallbackDefault()  = default;
			virtual void OnCallback(std::unique_ptr<Bee::Package> package) {}
	};

}

#endif
