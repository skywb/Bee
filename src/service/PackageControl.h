#ifndef PACKAGECONTROL_H
#define PACKAGECONTROL_H

#include "package/Package.h"

#include <queue>
#include <functional>
#include <map>
#include <iostream>
#include <mutex>

namespace Bee {

	class PackageCompleting
	{
	public:
		PackageCompleting (std::shared_ptr<Buffer> buf);
		PackageCompleting ();
		virtual ~PackageCompleting ();

		void Init(std::shared_ptr<Buffer> buf);
		bool AddBuffer(std::shared_ptr<Buffer> buf);
		std::unique_ptr<Package> GetPackage();
		bool Check();
		void OnBufferNotFound();
		size_t GetMaxPackageNumber() { return begin_number_+buf_count_-1; }
		std::unique_ptr<Package> OutTime();

	private:
		size_t begin_number_;
		size_t buf_count_;
		size_t cur_count_;
		std::vector<std::shared_ptr<Buffer>> buffers_;
		std::mutex mutex_;
		bool is_inited_ = false;
		bool is_callbacked_ = false;
	};

	class PackageControl {

	public:
		typedef std::function<void(std::unique_ptr<Package>)> PackageArrivedCallback;

	private:
		std::map<size_t, std::shared_ptr<PackageCompleting>> packages_;
		std::mutex mutex_packages_;
		PackageArrivedCallback OnPackageArrivedCallback_;
		size_t current_buffer_number_ = 1;
		//size_t min_pack_num_can_received_ = 0;

	public:
		PackageControl() {
			OnPackageArrivedCallback_ = [](std::unique_ptr<Package> package){return;};
	   	}
		virtual ~PackageControl() {}
		void OnReceivedBuffer(std::shared_ptr<Buffer> buffer);
		void OnBufferNotFound(size_t pack_num);
		std::vector<std::unique_ptr<Buffer>> SplitPackage(std::unique_ptr<Package> package);
		void SetPackageArrivedCallback(PackageArrivedCallback callback) { OnPackageArrivedCallback_ = callback; }
		size_t GetWaittingBufferNumber (const size_t max);
		void ClearOutTimePackage(const size_t min);
	private:
		std::shared_ptr<PackageCompleting> GetCompleting(const size_t beginNumber);
		std::shared_ptr<PackageCompleting> EmpleaceCompleting(std::shared_ptr<Buffer> buf);

	};

}

#endif
