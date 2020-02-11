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

		void InitHeader();
	public:
		static size_t GetMaxSizeOfNotSplit() { return kMaxDataSizeOneBuffer_; }

		Buffer(const uint8_t* buf, size_t size, size_t pack_num = 1, size_t begin = 1, size_t count = 1);
		//Buffer(std::unique_ptr<uint8_t[]> buf, size_t size);

		//return buffer data,  buffer header + user data
		uint8_t* GetBufferData();
		size_t GetBufferSize();

		// return user data
		uint8_t* GetData();
		size_t GetDataSize();

		void SetData(const uint8_t* buf, size_t size, size_t pack_num = 1, size_t begin = 1, size_t count = 1);

		void SetPackNum(const size_t pack_num) { pack_num_ = pack_num; }
		void SetBegin(const size_t begin) { begin_ = begin; }
		void SetCount(const size_t count) { count_ = count; }
		//void SetData(std::unique_ptr<uint8_t[]> buf, size_t size);
	};
}



#endif
