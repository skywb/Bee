#include "PackageControl.h"
#include "service/mlog.h"

using namespace Bee;

void PackageControl::OnReceivedBuffer(std::unique_ptr<Buffer> buffer) {
	if (!OnPackageArrivedCallback_) {
		return ;
	}
	auto begin_num = buffer->GetBufferHeader().begin;
	if (buffer->GetBufferHeader().count == 1) {
		PackageCompleting pack(std::move(buffer));;
		auto package_complete = pack.GetPackage();
		if (package_complete)
			OnPackageArrivedCallback_(std::move(pack.GetPackage()));
		return;
	} else {
		auto it = packages_.find(begin_num);
		if (it == packages_.end()) {
			packages_.emplace(buffer->GetBufferHeader().begin,
					std::make_unique<PackageCompleting>(std::move(buffer)));
		} else {
			bool ok = it->second->AddBuffer(std::move(buffer));
			if (ok) {
				auto package = it->second->GetPackage();
				if (package) {
					OnPackageArrivedCallback_(std::move(package));
				}
			}
		}
	}
}

void PackageControl::OnBufferNotFound(size_t pack_num) {
	LOG_WARN << "buffer not found";
}

std::vector<std::unique_ptr<Buffer>> PackageControl::SplitPackage(std::unique_ptr<Package> package) {
	std::vector<std::unique_ptr<Buffer>> buffers;
	auto* buf = package->GetData().get();
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
		buffers.emplace_back(std::move(buffer));
	}
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
}

PackageCompleting::~PackageCompleting() { }


bool PackageCompleting::AddBuffer(std::unique_ptr<Buffer> buf) {
	if (buf->GetBufferHeader().begin != begin_number_) return false;
	size_t cur = buf->GetBufferHeader().pack_num - begin_number_;
	if (cur > buf_count_ || buffers_.at(cur)) return false;
	buffers_.at(buf->GetBufferHeader().pack_num - begin_number_).swap(buf);
	++cur_count_;
	if (cur_count_ >= buf_count_) return true;
	return false;
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

