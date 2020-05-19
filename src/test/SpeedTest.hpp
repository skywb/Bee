#pragma once

#include "client/Bee.h"
#include <fstream>

class SpeedCalculate
{
public:
	SpeedCalculate () : 
		data_size_(0),
		begin_time_(std::chrono::system_clock::now()),
		switch_(false)	{
			Run();
	}
	virtual ~SpeedCalculate () {
		switch_ = true;
		th_.join();
	}

	void AddSize(const unsigned long long Bytes) {
		std::lock_guard<std::mutex> lock(mutex_);
		data_size_ += Bytes;
	}

private:
	void Callback() {
		while (true) {
			if (!switch_) return;
			std::unique_lock<std::mutex> lock(mutex_);
			auto now_time = std::chrono::system_clock::now();
			auto dt = std::chrono::duration_cast<std::chrono::milliseconds> (now_time - begin_time_);	
			if (dt.count() != 0) {
				if (data_size_ < dt.count() * 1024 * 2) 
					std::cout << data_size_  / dt.count() << "KB/s" << std::endl;
				else
					std::cout << data_size_  / dt.count() / 1024 << "MB/s" << std::endl;
				begin_time_ = now_time;
				data_size_ = 0;
			}
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}
	void Run() {
		switch_ = true;
		std::thread th(std::thread(std::bind(&SpeedCalculate::Callback, this)));
		th_.swap(th);
	}

	unsigned long long data_size_;
	std::mutex mutex_;
	std::chrono::time_point<std::chrono::system_clock> begin_time_;
	std::thread th_;
	bool switch_;
};

class SpeedTest : public Bee::PackageArrivedCallback {
public:
	SpeedTest () = default;
	virtual ~SpeedTest () = default;
	virtual void OnCallback(std::unique_ptr<Bee::Package> package) override;
private:
	SpeedCalculate speed_;
};

void SpeedTest::OnCallback(std::unique_ptr<Bee::Package> package) {
//	if (package->GetStat() == Bee::Package::SUCCEE)
		speed_.AddSize(package->GetSize());
//	else 
//	{
//		std::cout << "recive faild" << std::endl;
//	}
}
