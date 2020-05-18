#include "PackageControl.h"
#include "log/mlog.hpp"
#include "boost/progress.hpp"

using namespace Bee;

void PackageControl::OnReceivedBuffer(std::shared_ptr<Buffer> buffer) {
	if (!OnPackageArrivedCallback_) {
		return ;
	}
	auto begin_num = buffer->GetBufferHeader().begin;
	auto completing = GetCompleting(begin_num);
	if (completing) {
		completing->AddBuffer(buffer);
	} else {
		completing = EmpleaceCompleting(std::move(buffer));
	}
	if (completing->Check()) {
		auto package = std::move(completing->GetPackage());
		if (package) {
			if (OnPackageArrivedCallback_)
				OnPackageArrivedCallback_(std::move(package));
			std::lock_guard<std::mutex> lock(mutex_packages_);
			packages_.erase(begin_num);
		}
	}
}

bool PackageCompleting::Check() {
	std::lock_guard<std::mutex> lock(mutex_);
	if (is_callbacked_) {
		return false;
	}
	return cur_count_ >= buf_count_;
}

size_t PackageControl::GetBufferNumber(const size_t cnt) {
	std::lock_guard<std::mutex> lock(mutex_buffer_number_);
	size_t beg_num = current_buffer_number_;
	current_buffer_number_ += cnt;
	return beg_num;
}

std::shared_ptr<PackageCompleting> 
PackageControl::GetCompleting(const size_t beginNumber) {
	std::lock_guard<std::mutex> lock(mutex_packages_);
	auto it = packages_.find(beginNumber);
	if (it == packages_.end()) {
	   	return nullptr;
	}
	else return it->second;
}

std::shared_ptr<PackageCompleting> 
PackageControl::EmpleaceCompleting(std::shared_ptr<Buffer> buf) {
	size_t begin_number = buf->GetBufferHeader().begin;
	std::lock_guard<std::mutex> lock(mutex_packages_);
	auto it = packages_.find(begin_number);
	if (it == packages_.end()) {
		packages_.emplace(begin_number, std::make_shared<PackageCompleting>());
		it = packages_.find(begin_number);
		it->second->Init(buf);
	}
	return it->second;
}


void PackageControl::OnBufferNotFound(size_t pack_num) {
	std::lock_guard<std::mutex> lock(mutex_packages_);
	auto it = packages_.upper_bound(pack_num);
	if (it == packages_.end()) return;
	--it;
	if (it == packages_.end() || it->first < pack_num) return;
	if (it->second) {
		it->second->OutTime();
	}
	//for (auto it = packages_.begin(); it != packages_.end(); ++it) {
	//	if (it->first > pack_num) return;
	//	if (it->second->GetMaxPackageNumber() < pack_num) {
	//		auto package = it->second->OutTime();
	//		if (package) {
	//			if (OnPackageArrivedCallback_)
	//				OnPackageArrivedCallback_(std::move(package));
	//		}
	//	}
	//}
}

std::unique_ptr<Package> 
PackageCompleting::GetPackage() {
	std::lock_guard<std::mutex> lock(mutex_);
	if (cur_count_ < buf_count_ || is_callbacked_) {
		LOG_DEBUG << "current buffer count is " << cur_count_ 
			<< " sum is " << buf_count_;
		return nullptr;
	}
	is_callbacked_ = true;
	size_t size = 0;
	for (auto& i : buffers_) {
		size += i->GetDataSize();
	}
	auto package = std::make_unique<Package>();
	auto data = std::make_unique<uint8_t[]> (size);
	auto* buf = data.get();
	for (auto& i : buffers_) {
		memcpy(buf, i->GetData(), i->GetDataSize());
		buf += i->GetDataSize();
		i.reset();
	}
	package->SetData(std::move(data), size);
	return std::move(package);
}

std::unique_ptr<Package> PackageCompleting::OutTime() {
	std::lock_guard<std::mutex> lock(mutex_);
	if (is_callbacked_) {
		return nullptr;
	}
	LOG_DEBUG << "Out Time: current buffer count is " << cur_count_ << " sum is " << buf_count_;
	is_callbacked_ = true;
	size_t size = 0;
	auto package = std::make_unique<Package>();
	for (auto& i : buffers_) {
		if (i)
			i.reset();
	}
	package->SetOutTime();
	return std::move(package);
}

std::vector<std::shared_ptr<Buffer>> 
PackageControl::SplitPackage(std::shared_ptr<Package> package) {
	std::vector<std::shared_ptr<Buffer>> buffers;
	auto buf = package->GetData();
	size_t size = package->GetSize();
	if (size <= 0) return std::move(buffers);
	int buffer_cnt = size / Buffer::GetMaxSizeOfNotSplit();
	if (buffer_cnt * Buffer::GetMaxSizeOfNotSplit() < size) ++buffer_cnt;
	size_t begin_num = GetBufferNumber(buffer_cnt);
	size_t pack_num = begin_num;
	while (size > 0) {
		auto buffer = std::make_shared<Buffer> ();
		if (size >= Buffer::GetMaxSizeOfNotSplit()) {
			buffer->SetData(buf, Buffer::GetMaxSizeOfNotSplit());
			buf += Buffer::GetMaxSizeOfNotSplit();
			size -= Buffer::GetMaxSizeOfNotSplit();
		} else {
			buffer->SetData(buf, size);
			size = 0;
		}
		buffer->SetPackNum(pack_num);
		++pack_num;
		buffer->SetBegin(begin_num);
		buffer->SetCount(buffer_cnt);
		buffer->SetBufferType(BufferType::DATA);
		buffers.push_back(buffer);
	}
	return std::move(buffers);
}

PackageCompleting::PackageCompleting(std::shared_ptr<Buffer> buf) :
	buffers_(buf->GetBufferHeader().count){
	if (!buf)
		return;
	begin_number_ = buf->GetBufferHeader().begin;
	buf_count_ = buf->GetBufferHeader().count;
	cur_count_ = 1;
	buffers_.at(buf->GetBufferHeader().pack_num - begin_number_) = buf;
	is_inited_ = true;
}

PackageCompleting::PackageCompleting() {
}

PackageCompleting::~PackageCompleting() { }

void PackageCompleting::Init(std::shared_ptr<Buffer> buf) {
	if (!buf)
		return;
	std::lock_guard<std::mutex> lock(mutex_);
	if (is_inited_) {
		AddBuffer(buf);
		return;
	}
	buffers_.resize(buf->GetBufferHeader().count);
	begin_number_ = buf->GetBufferHeader().begin;
	buf_count_ = buf->GetBufferHeader().count;
	cur_count_ = 1;
	buffers_.at(buf->GetBufferHeader().pack_num - begin_number_) = buf;
	is_inited_ = true;
}


bool PackageCompleting::AddBuffer(std::shared_ptr<Buffer> buf) {
	if (buf->GetBufferHeader().begin != begin_number_) return false;
	size_t cur = buf->GetBufferHeader().pack_num - begin_number_;
	if (cur >= buf_count_) return false;
	std::unique_lock<std::mutex> lock(mutex_);
    if (buffers_.at(cur)) return false;
	buffers_.at(buf->GetBufferHeader().pack_num - begin_number_) = buf;
	++cur_count_;
	lock.unlock();
	return true;
}



//size_t PackageControl::GetWaittingBufferNumber(const size_t max) {
//	std::lock_guard<std::mutex> lock(mutex_packages_);
//	if (packages_.empty()) {
//		return max;
//	}
//	return  packages_.crbegin()->second->GetMaxPackageNumber();
//}
//
//void PackageControl::ClearOutTimePackage(const size_t min) {
//	std::vector<std::unique_ptr<Package>> outtime_packages;
//	std::unique_lock<std::mutex> lock(mutex_packages_);
//	for (auto it = packages_.begin(); it != packages_.end(); ) {
//		if (it->second->GetMaxPackageNumber() < min) {
//			auto package = std::move(it->second->OutTime());
//			outtime_packages.push_back(std::move(package));
//			it = packages_.erase(it);
//		} else {
//			++it;
//		}
//	}
//	lock.unlock();
//	for (auto& i : outtime_packages) {
//			if (OnPackageArrivedCallback_)
//				OnPackageArrivedCallback_(std::move(i));
//	}
//}
