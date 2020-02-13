#ifndef PACKAGECONTROL_H
#define PACKAGECONTROL_H

#include "package/Package.h"

#include <queue>
#include <functional>
#include <map>

namespace Bee {
	class PackageControl {

	public:
		typedef std::function<void(std::unique_ptr<Package>)> PackageArrivedCallback;

	private:
		std::map<int, std::unique_ptr<Buffer>> pack_received_;
		PackageArrivedCallback OnPackageArrivedCallback_;
		std::queue<Package> package_completed_;

	public:
		PackageControl() {}
		virtual ~PackageControl() {}
		void OnReceivedPack(std::unique_ptr<Buffer> buffer);
		void OnPackNotFound(size_t pack_num);

		void BuildUpPackage();

		std::vector<std::unique_ptr<Buffer>>&& SplitPackage(std::unique_ptr<Package> package);

		void SetPackageArrivedCallback(PackageArrivedCallback callback) { OnPackageArrivedCallback_ = callback; }
	};
}

#endif
