#ifndef CONNECTER_H
#define CONNECTER_H

namespace RecoverManager {
	class Connecter {

	private:
		std::unique_ptr<RecoverManager::Service> serviece_;

	public:
		void SetLocalIPAndPort();

		void SetRemoteIPAndPort();

		void SendPackage();

		void ReceivePackage();

		void SetClientHeartRate();

		void operation();
	};
}

#endif
