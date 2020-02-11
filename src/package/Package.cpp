#include "Package.h"

#include <cstring>

using namespace Bee;

std::unique_ptr<uint8_t[]> Package::GetData() {
	// TODO - implement Package::GetData
	throw "Not yet implemented";
}

void Package::SetData(std::unique_ptr<uint8_t[]> data, size_t size) {
	// TODO - implement Package::SetData
	throw "Not yet implemented";
}

size_t Package::GetSize() {
	// TODO - implement Package::GetSize
	throw "Not yet implemented";
}

Buffer::Buffer(const uint8_t* buf, size_t size, size_t pack_num, size_t begin, size_t count)
	:data_(nullptr),
	size_(size),
	pack_num_(pack_num),
	begin_(begin),
	count_(count) {
	if (size_ == 0) return;
	size_ = size;
	data_ = new uint8_t[size_+kBufferHeaderSize_];
	memcpy(data_+kBufferHeaderSize_, buf, size_);
	InitHeader();
}

void Buffer::InitHeader() {
	if (data_ == nullptr) return;
	decltype(data_) buf = data_;
	memcpy(buf, &pack_num_, sizeof(pack_num_));
	buf+= sizeof(pack_num_);
	memcpy(buf, &begin_, sizeof(begin_));
	buf+=sizeof(begin_);
	memcpy(buf, &count_, sizeof(count_));
	buf+= sizeof(count_);
	memcpy(buf, &size_, sizeof(size_));
}



uint8_t* Buffer::GetBufferData() {
	InitHeader();
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

void Buffer::SetData(const uint8_t* data, size_t size, size_t pack_num, size_t begin, size_t count) {
	if (data_) delete  data_;
	size_ = size;
	pack_num_ = pack_num;
	begin_ = begin;
	count_ = count;
	data_ = new uint8_t[size_+kBufferHeaderSize_];
	memcpy(data_+kBufferHeaderSize_, data, size_);
}
