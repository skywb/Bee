#ifndef PACKAGECONTROL_H
#define PACKAGECONTROL_H

#include "package/Package.h"

#include <queue>
#include <functional>
#include <map>
#include <iostream>

namespace Bee {

	class PackageCompleting
	{
	public:
		PackageCompleting (std::unique_ptr<Buffer> buf);
		virtual ~PackageCompleting ();

		bool AddBuffer(std::unique_ptr<Buffer> buf);
		std::unique_ptr<Package> GetPackage();

	private:
		size_t begin_number_;
		size_t buf_count_;
		size_t cur_count_;
		std::vector<std::unique_ptr<Buffer>> buffers_;
	};

	class PackageControl {

	public:
		typedef std::function<void(std::unique_ptr<Package>)> PackageArrivedCallback;

	private:
		std::map<size_t, std::unique_ptr<PackageCompleting>> packages_;
		PackageArrivedCallback OnPackageArrivedCallback_;
		size_t current_buffer_number_ = 0;

	public:
		PackageControl() { }
		virtual ~PackageControl() {}
		void OnReceivedBuffer(std::unique_ptr<Buffer> buffer);
		void OnBufferNotFound(size_t pack_num);
		std::vector<std::unique_ptr<Buffer>> SplitPackage(std::unique_ptr<Package> package);
		void SetPackageArrivedCallback(PackageArrivedCallback callback) { OnPackageArrivedCallback_ = callback; }
	};

}

#endif
