#include "Package.h"

#include <cstring>

using namespace Bee;

std::unique_ptr<void*> Package::GetData() {
	// TODO - implement Package::GetData
	throw "Not yet implemented";
}

void Package::SetData(std::unique_ptr<void[]> data, size_t size) {
	// TODO - implement Package::SetData
	throw "Not yet implemented";
}

size_t Package::GetSize() {
	// TODO - implement Package::GetSize
	throw "Not yet implemented";
}

Buffer::Buffer(const uint8_t* buf /*copy this buf*/, size_t size)
	:data_(nullptr),
	size_(size),
	pack_num_(0),
	begin_(0),
	count_(1) {
	if (size_ == 0) return;
	data_ = new uint8_t[size_+kBufferHeaderSize_];
	InitBuffer(buf, size);
}

void Buffer::InitBuffer(const uint8_t* data, size_t size) {
	if (data_ == nullptr) return;
	decltype(data_) buf;
	memcpy(buf, &pack_num_, sizeof(pack_num_));
	buf+= sizeof(pack_num_);
	memcpy(buf, &begin_, sizeof(begin_));
	buf+=sizeof(begin_);
	memcpy(buf, &count_, sizeof(count_));
	buf+= sizeof(count_);
	memcpy(buf, &size_, sizeof(size_));
	buf+= sizeof(size_);
	memcpy(buf, data, size);
}



uint8_t* Buffer::GetBufferData() {
	return data_;
}

size_t Buffer::GetBufferSize() {
	return size_+kBufferHeaderSize_;
}

uint8_t* Buffer::GetData() {
	return data_+kBufferHeaderSize_;
}

size_t Buffer::GetDataSize() {
	return size_;
}

void Buffer::SetData(const uint8_t* data, size_t size) {
	if (data_) delete  data_;
	data_ = new uint8_t[size_];
	InitBuffer(data, size);
}
