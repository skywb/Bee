#ifndef PACKAGE_H
#define PACKAGE_H

#include <memory>

namespace Bee {
	class Package {
	private:
		std::size_t size_;
		std::unique_ptr<uint8_t[]> data_;

	public:
		std::unique_ptr<uint8_t[]> GetData();

		void SetData(std::unique_ptr<uint8_t[]> data, size_t size);

		size_t GetSize();
	};


	class Buffer {
	private:
		uint8_t* data_;
		size_t pack_num_;
		size_t begin_;
		size_t count_;
		size_t size_;

		static const size_t kBufferHeaderSize_ = 
			sizeof(pack_num_) + sizeof(begin_) + sizeof(count_) + sizeof(size_);
		static const size_t kMaxDataSizeOneBuffer_ = 1472 - kBufferHeaderSize_;

		void InitBuffer(const uint8_t* data, size_t size);
	public:
		static size_t GetMaxSizeOfNotSplit() { return kMaxDataSizeOneBuffer_; }

		Buffer(const uint8_t* buf, size_t size);
		//Buffer(std::unique_ptr<uint8_t[]> buf, size_t size);

		//return buffer data,  buffer header + user data
		uint8_t* GetBufferData();
		size_t GetBufferSize();

		// return user data
		uint8_t* GetData();
		size_t GetDataSize();

		void SetData(const uint8_t* data, size_t size);
		//void SetData(std::unique_ptr<uint8_t[]> buf, size_t size);
	};
}



#endif
