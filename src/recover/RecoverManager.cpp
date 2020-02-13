#include "RecoverManager.h"

using namespace Bee;

void RecoverManager::PackageArrived(size_t package_num) {
	if (is_first_pack_num) {
		min_pack_num_ = package_num;
		max_pack_num_ = package_num;
		return;
	}

	if (package_num < min_pack_num_) {
		return;
	}

	if (package_num > max_pack_num_) {
		{
			std::lock_guard<std::mutex> lock(mutex_recover_wait_);
			auto max_pack_it = recover_wait_.crbegin();
			size_t max_pack = max_pack_num_;
			if (max_pack_it != recover_wait_.crend()) {
				max_pack = max_pack_it->first;
			}
			for (size_t i = max_pack+1; i<package_num; ++i) {
				recover_wait_.emplace(i, std::chrono::system_clock::now());
			}
		}
		max_pack_num_ = package_num;
	} else {
		{
			std::lock_guard<std::mutex> lock(mutex_recover_wait_);
			auto it = recover_wait_.find(package_num);
			if (it == recover_wait_.end()) {
				return;
			} else {
				//TODO: update rtt
				recover_wait_.erase(it);
				if (recover_wait_.empty()) {
					oldest_nack_pack_num_ = max_pack_num_;
				} else {
					oldest_nack_pack_num_ = recover_wait_.begin()->first;
				}
			}
		}
	}

	if (min_pack_num_ < oldest_nack_pack_num_) 
		min_pack_num_ = oldest_nack_pack_num_;
}

void RecoverManager::AddPackRecord(std::shared_ptr<Buffer> buf) {
	AddPackToHistroy(buf->GetBufferHeader().pack_num, buf);
	ClearOutTimeHistory();
}

void RecoverManager::AddPackToHistroy(size_t package_num, std::shared_ptr<Buffer> pack_data) {
	std::lock_guard<std::mutex> lock(mutex_package_history_);
	package_history_.emplace(package_num, pack_data);
}


RecoverManager::RecoverManager(boost::asio::io_service& service, SenderCallback sender)
	: service_(service),
	timer_pack_outtime_(service_),
	sender_(sender) {

}

RecoverManager::~RecoverManager() {

}

void RecoverManager::ClearOutTimeHistory() {
	int cnt = 1000 - package_history_.size();
	if (cnt > 0)  {
		auto end = package_history_.begin();
		for (int i = 0; i < cnt; ++i) {
			end++;
		}
		package_history_.erase(package_history_.begin(), end);
	}
}

std::shared_ptr<Buffer> RecoverManager::GetHistory(size_t pack_num) {
	std::lock_guard<std::mutex> lock(mutex_package_history_);
	auto it = package_history_.find(pack_num);
	if (it == package_history_.end()) {
		return nullptr;
	}
	return it->second;
}

void RecoverManager::NACKReceived(size_t pack_num, UDPEndPoint endpoint) {
	auto pack = GetHistory(pack_num);
	sender_(pack, endpoint);
}
