#include "PackageControl.h"
#include "service/mlog.h"

using namespace Bee;

void PackageControl::OnReceivedBuffer(std::unique_ptr<Buffer> buffer) {
	if (!OnPackageArrivedCallback_) {
		return ;
	}
	auto begin_num = buffer->GetBufferHeader().begin;
	auto completing = GetCompleting(begin_num);
	if (completing) {
		completing->AddBuffer(std::move(buffer));
	} else {
		completing = EmpleaceCompleting(begin_num);
		completing->Init(std::move(buffer));
	}
	if (completing->Check()) {
		auto package = completing->GetPackage();
		if (package) {
			LOG_INFO << "package complete";
			OnPackageArrivedCallback_(std::move(package));
		}
	}
}

bool PackageCompleting::Check() {
	std::lock_guard<std::mutex> lock(mutex_);
	return cur_count_ >= buf_count_;
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
PackageControl::EmpleaceCompleting(const size_t beginNumber) {
	std::lock_guard<std::mutex> lock(mutex_packages_);
	auto it = packages_.find(beginNumber);
	if (it == packages_.end()) {
		packages_.emplace(beginNumber, std::make_shared<PackageCompleting>());
		it = packages_.find(beginNumber);
	}
	return it->second;
}


void PackageControl::OnBufferNotFound(size_t pack_num) {
	LOG_WARN << "buffer not found";
}

std::vector<std::unique_ptr<Buffer>> PackageControl::SplitPackage(std::unique_ptr<Package> package) {
	std::vector<std::unique_ptr<Buffer>> buffers;
	auto buf = package->GetData();
	size_t size = package->GetSize();
	if (size <= 0) return std::move(buffers);
	int buffer_cnt = size / Buffer::GetMaxSizeOfNotSplit();
	if (buffer_cnt * Buffer::GetMaxSizeOfNotSplit() < size) ++buffer_cnt;
	size_t begin_num = current_buffer_number_;
	while (size > 0) {
		auto buffer = std::make_unique<Buffer> ();
		if (size >= Buffer::GetMaxSizeOfNotSplit()) {
			buffer->SetData(buf, Buffer::GetMaxSizeOfNotSplit());
			buf += Buffer::GetMaxSizeOfNotSplit();
			size -= Buffer::GetMaxSizeOfNotSplit();
		} else {
			buffer->SetData(buf, size);
			size = 0;
		}
		buffer->SetPackNum(current_buffer_number_);
		++current_buffer_number_;
		buffer->SetBegin(begin_num);
		buffer->SetCount(buffer_cnt);
		buffer->SetBufferType(BufferType::DATA);
		buffers.emplace_back(std::move(buffer));
	}
	LOG_INFO << "sum of buffers " << buffers.size();
	return std::move(buffers);
}

PackageCompleting::PackageCompleting(std::unique_ptr<Buffer> buf) :
	buffers_(buf->GetBufferHeader().count){
	if (!buf)
		return;
	begin_number_ = buf->GetBufferHeader().begin;
	buf_count_ = buf->GetBufferHeader().count;
	cur_count_ = 1;
	buffers_.at(buf->GetBufferHeader().pack_num - begin_number_).swap(buf);
	is_inited_ = true;
}
PackageCompleting::PackageCompleting() {
}

PackageCompleting::~PackageCompleting() { }

void PackageCompleting::Init(std::unique_ptr<Buffer> buf) {
	if (!buf)
		return;
	std::lock_guard<std::mutex> lock(mutex_);
	if (is_inited_) {
		AddBuffer(std::move(buf));
		return;
	}
	buffers_.resize(buf->GetBufferHeader().count);
	begin_number_ = buf->GetBufferHeader().begin;
	buf_count_ = buf->GetBufferHeader().count;
	cur_count_ = 1;
	buffers_.at(buf->GetBufferHeader().pack_num - begin_number_).swap(buf);
	is_inited_ = true;
}


bool PackageCompleting::AddBuffer(std::unique_ptr<Buffer> buf) {
	if (buf->GetBufferHeader().begin != begin_number_) return false;
	size_t cur = buf->GetBufferHeader().pack_num - begin_number_;
	if (cur > buf_count_) return false;
	std::unique_lock<std::mutex> lock(mutex_);
    if (buffers_.at(cur)) return false;
	buffers_.at(buf->GetBufferHeader().pack_num - begin_number_).swap(buf);
	++cur_count_;
	lock.unlock();
	LOG_INFO << "current buffers is " << cur_count_ << " sum buffers is " << buf_count_;
	return true;
}



std::unique_ptr<Package> PackageCompleting::GetPackage() {
	if (cur_count_ < buf_count_) {
		return nullptr;
	}
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
	}

	package->SetData(std::move(data), size);
	return std::move(package);
}

