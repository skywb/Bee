#include "Package.h"
#include "log/mlog.hpp"

#include <cstring>

using namespace Bee;

uint8_t*const  Package::GetData() {
	//return std::move(data_);
	return data_.get();
}

void Package::SetData(std::unique_ptr<uint8_t[]> data, size_t size) {
	size_ = size;
	data_ = std::move(data);
	stat_ = SUCCEE;
}

void Package::SetData(const char* buf, size_t size) {
	data_ = std::make_unique<uint8_t[]> (size);
	size_ =  size;
	memcpy(data_.get(), buf, size);
	stat_ = SUCCEE;
}

size_t Package::GetSize() {
	return size_;
}

Buffer::Buffer(const uint8_t* buf, size_t size)
	: data_(nullptr) {
		if (size < kBufferHeaderSize_ || size > 1500) return;
		if (buf == nullptr) {
			//data_ = new uint8_t[kBufferHeaderSize_];
		} else {
			data_ = new uint8_t[size];
			memcpy(data_, buf, size);
			memcpy(&header_, data_, sizeof(header_));
		}
}

Buffer::~Buffer() {
	if (data_) delete[] data_;
}

void Buffer::SetData(uint8_t*const  data, size_t size) {
	if (data_) delete  data_;
	header_.size = size;
	data_ = new uint8_t[header_.size+kBufferHeaderSize_];
	if (header_.size == 0) {
		return;
	} else {
		memcpy(data_+kBufferHeaderSize_, data, header_.size);
	}
}

uint8_t* Buffer::GetBufferData() {
	InitHeader();
	return data_;
}

size_t Buffer::GetBufferSize() {
	return header_.size+kBufferHeaderSize_;
}

uint8_t* Buffer::GetData() {
	return data_+kBufferHeaderSize_;
}

size_t Buffer::GetDataSize() {
	return header_.size;
}


//void BufferFromPackage::SetData(uint8_t *const  data, size_t size)  {
//	uint8_t *const buf = package_->GetData();
//	if (header_.pack_num - header_.begin > package_->GetBufferCount()) return;
//	auto foo = package_->GetBuffer(header_.pack_num - header_.begin);
//	data_ = foo.first;
//	header_.size = foo.second;
//}
//
//uint8_t* BufferFromPackage::GetBufferData()  {
//	if (header_.pack_num == 0) return nullptr;
//	return data_;
//}
