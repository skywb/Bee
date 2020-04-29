#include "RecoverManager.h"
#include "log/mlog.hpp"
#include <mutex>

using namespace Bee;

RecoverManager::RecoverManager(boost::asio::io_service& service, Interface* sender) : 
	service_(service),
	sender_(sender),
	timer_NACK_tracer(service),
	rtt_(400) {
		size_t cur_rtt = rtt_;
		min_pack_num_ = 0;
		max_pack_num_ = 0;
		timer_NACK_tracer.expires_from_now(boost::posix_time::milliseconds(cur_rtt));
		timer_NACK_tracer.async_wait(std::bind(&RecoverManager::NackTrackerHandler, this, std::placeholders::_1));
   	}

RecoverManager::~RecoverManager() {
	timer_NACK_tracer.cancel();
}

bool RecoverManager::PackageArrived(size_t package_num) {
	if (is_first_pack_num) {
		is_first_pack_num = false;
		min_pack_num_ = package_num;
		max_pack_num_ = package_num;
		return true;
	}

	if (package_num < min_pack_num_ && min_pack_num_ + 10000 > min_pack_num_) {
		return false;
	}

	std::lock_guard<std::mutex> lock(mutex_recover_wait_);
	if (package_num > max_pack_num_) { //更大的包需要接收
		for (size_t i = max_pack_num_+1; i<package_num; ++i) {
			recover_wait_.emplace(i, std::chrono::system_clock::now());
		}
		max_pack_num_ = package_num;
	} else {
		auto it = recover_wait_.find(package_num);
		if (it == recover_wait_.end()) {
			return false;
		} else {
			recover_wait_.erase(it);
			if (recover_wait_.empty()) {
				min_pack_num_ = max_pack_num_+1;
			} else {
				min_pack_num_ = recover_wait_.begin()->first;
			}
		}
	}
	return true;
}

void RecoverManager::AddPackRecord(std::shared_ptr<Buffer> buf) {
	AddPackToHistroy(buf->GetBufferHeader().pack_num, buf);
	ClearOutTimeHistory();
}

void RecoverManager::AddPackToHistroy(size_t package_num, std::shared_ptr<Buffer> pack_data) {
	std::lock_guard<std::mutex> lock(mutex_package_history_);
	if (package_history_.find(package_num) == package_history_.end()) {
		auto result = package_history_.emplace(std::make_pair(package_num, pack_data));
		if (!result.second) {
			LOG_DEBUG << "emplace error, package number is " << package_num;
		}
	}
}

void RecoverManager::ClearOutTimeHistory() {
	int cnt = package_history_.size() - history_max_len_ ;
	if (cnt > 0)  {
		std::lock_guard<std::mutex> lock(mutex_package_history_);
		auto end = package_history_.begin();
		std::advance(end, cnt);
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
	if (!pack) {
		pack = Buffer::MakeBuffer();
		pack->SetBufferType(BufferType::BUFFER_NOT_FOUND);
	}
	sender_->SendPackageTo(pack, endpoint);
}

void RecoverManager::NackTrackerHandler(const boost::system::error_code& error) {
	size_t cur_rtt = rtt_;
	auto now = std::chrono::system_clock::now();
	std::vector<int> nacks;
	std::unique_lock<std::mutex> lock(mutex_recover_wait_);
	auto it = recover_wait_.begin();
	while (it != recover_wait_.end()) {
		if (it->second + std::chrono::milliseconds(cur_rtt*4) < now) {
			it = recover_wait_.erase(it);
			/*! TODO: 处理超时的包
			*  \todo 处理超时的包
			*/
		} else if (it->second + std::chrono::milliseconds(cur_rtt) < now) {
			nacks.push_back(it->first);
			//sender_->SendNack(it->first);
			++it;
		} else {
			++it;
		}
	}
	cur_rtt = rtt_;
	lock.unlock();
	for (auto i : nacks) {
		sender_->SendNack(i);
	}
	timer_NACK_tracer.expires_from_now(boost::posix_time::milliseconds(cur_rtt));
	timer_NACK_tracer.async_wait(std::bind(&RecoverManager::NackTrackerHandler, this, std::placeholders::_1));
}

bool RecoverManager::WaitPackage(const size_t min_packnum, const size_t max_packnum) {
	std::lock_guard<std::mutex> lock(mutex_recover_wait_);
	if (min_packnum < min_pack_num_) return false;
	if (max_packnum > max_pack_num_) {
		for (size_t i = max_pack_num_+1; i<=max_packnum; ++i) {
			recover_wait_.emplace(i, std::chrono::system_clock::now());
		}
		max_pack_num_ = max_packnum;
	}
	return true;
}
