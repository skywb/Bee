#include "Package.h"
#include "service/mlog.h"

#include <cstring>

using namespace Bee;

const uint8_t* Package::GetData() {
	//return std::move(data_);
	return data_.get();
}

void Package::SetData(std::unique_ptr<uint8_t[]> data, size_t size) {
	size_ = size;
	data_ = std::move(data);
	stat_ = COMPLETED;
}

void Package::SetData(const char* buf, size_t size) {
	data_ = std::make_unique<uint8_t[]> (size);
	size_ =  size;
	memcpy(data_.get(), buf, size);
	stat_ = COMPLETED;
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

void Buffer::SetData(const uint8_t* data, size_t size) {
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

