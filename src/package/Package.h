#ifndef PACKAGE_H
#define PACKAGE_H

#include <memory>
#include <cstring>
#include <bitset>

namespace Bee {

#pragma pack(push, 4)

	enum BufferType {
		DATA = 1,
		NACK = 2,
		BUFFER_NOT_FOUND = 3,
		HEARTBEAT = 4,
		SYNC_HEATBEAT = 5
	};
	struct BufferHeader {
		size_t pack_num = 0;
		BufferType type;
		size_t begin = 0;
		size_t count = 0;
		union {
			size_t size;
			size_t heartbeat_rate;
		};
	};
#pragma pack(pop)

	class Buffer {
	private:
		uint8_t* data_;
		BufferHeader header_;

		static const size_t kBufferHeaderSize_ = sizeof(BufferHeader);
		static const size_t kMaxDataSizeOneBuffer_ = 1472 - kBufferHeaderSize_;

		void InitHeader() { memcpy(data_, &header_, sizeof(header_)); }

	public:
		static size_t GetMaxSizeOfNotSplit() { return kMaxDataSizeOneBuffer_; }
		static std::unique_ptr<Buffer> MakeBuffer() { return std::make_unique<Buffer> (); }

		Buffer(const uint8_t* buf = nullptr, size_t size = kBufferHeaderSize_);
		~Buffer();
		//Buffer(std::unique_ptr<uint8_t[]> buf, size_t size);
		void SetData(const uint8_t* buf, size_t size);

		//return buffer data,  buffer header + user data
		uint8_t* GetBufferData();
		size_t GetBufferSize();

		// return user data
		uint8_t* GetData();
		size_t GetDataSize();


		void SetPackNum(const size_t pack_num) { header_.pack_num = pack_num; }
		void SetBegin(const size_t begin) { header_.begin = begin; }
		void SetCount(const size_t count) { header_.count = count; }
		void SetBufferType(BufferType type) { header_.type = type; }
		void SetHeartRate(const size_t rate) { header_.heartbeat_rate = rate; }
		const BufferHeader& GetBufferHeader() { return header_; }
	};

	class Package {
	private:
		std::size_t size_;
		std::unique_ptr<uint8_t[]> data_;
		bool is_complete_package_ = false;
	public:
		static size_t GetMaxSizeOfNotSplit() { return Buffer::GetMaxSizeOfNotSplit(); }

		const uint8_t* GetData();

		void SetData(std::unique_ptr<uint8_t[]> data, size_t size);
		void SetData(const char* buf, size_t size);

		size_t GetSize();
	};
}



#endif
