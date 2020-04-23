#ifndef CONNECTER_H
#define CONNECTER_H

#include "service/Service.h"
#include "package/Package.h"

namespace Bee {
	class Connecter {

	private:
		std::unique_ptr<Bee::Service> service_;
		std::size_t thread_count_ = 5;

	public:
		typedef std::function<void(std::unique_ptr<Package>)> Callback;
		Connecter();
		~Connecter();
		void SetLocalIPAndPort(const std::string IP, const short port);
		void SetThreadCount(const size_t thread_count);
		void AddService(const std::string IP, const short port);
		void RemoveService(const std::string IP, const short port);
		void AddClient(const std::string IP, const short port);
		void RmoveClient(const std::string IP, const short port);
		void SendPackage(std::unique_ptr<Package> package);
		bool RequestToServeice(std::unique_ptr<Package> package);
		bool RequestToServeice(std::unique_ptr<Package> package, const std::string IP, const short port);
		void SetPackageArrivedCallback(Callback callback);
		void SetPackageArrivedCallback(std::unique_ptr<PackageArrivedCallback> callback);
		void SetPackageSendedCallback(Callback callback);
		void SetPackageSendedCallback(std::unique_ptr<PackageSendedCallback> callback);

		// get receiver current RTT
		size_t GetRTT();

		// 设置将接收到的数据转发
		void SetTranspond(bool transpond);

		// set sender heater
		void SetHeartRate(const std::size_t ms);
		// set sender Buffer alive time
		void SetBufferOutTime(int ms); //ms
		int GetBufferOutTime();

		void SetBufferHistorySize(const size_t size); //ms
	};
}

#endif
