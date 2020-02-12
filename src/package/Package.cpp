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

Buffer::Buffer(const uint8_t* buf, size_t size)
	:data_(nullptr) {
		if (size < kBufferHeaderSize_) return;
		data_ = new uint8_t[size];
		memcpy(&header_, data_, sizeof(header_));
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

