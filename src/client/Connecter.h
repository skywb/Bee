#ifndef CONNECTER_H
#define CONNECTER_H

#include "service/Service.h"

namespace Bee {
	class Connecter {

	private:
		std::unique_ptr<Bee::Service> service_;
		std::size_t thread_count_ = 2;

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
		void RequestToServeice(std::unique_ptr<Package> package);
		void RequestToServeice(std::unique_ptr<Package> package, const std::string IP, const short port);
		void SetPackageArrivedCallback(Callback callback);

		void GetRTT();

		void SetTranspond(bool transpond);

		// set sender heater
		void SetHeartRate(const std::size_t ms);
		void SetBufferOutTime(int ms); //ms
		int GetBufferOutTime();
	};
}

#endif
