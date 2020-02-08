#ifndef PACKAGECONTROL_H
#define PACKAGECONTROL_H

#include "package/Package.h"

namespace Bee {
	class PackageControl {

	private:
		std::map<int, std::unique_ptr<Buffer>> pack_received_;
		std::funtional<void(std::unique_ptr<Package>)> OnPackageArrivedCallback_;
		std::queue<Package> package_completed_;

	public:
		void OnReceivedPack(std::unique_ptr<Buffer> buffer);

		void BuildUpPackage();

		std::vector<Buffer>&& SplitPackage(Package package);
	};
}

#endif
