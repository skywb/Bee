#ifndef PACKAGE_H
#define PACKAGE_H

#include <memory>
#include <cstring>
#include <bitset>
#include <vector>
#include <functional>

namespace Bee {

#pragma pack(push, 4)

	enum BufferType {
		DATA = 1,
		NACK = 2,
		BUFFER_NOT_FOUND = 3,
		HEARTBEAT = 4,
		SYNC_HEATBEAT = 5,
		REQUEST = 6
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
	protected:
		uint8_t* data_;
		BufferHeader header_;
	private:
		static const size_t kBufferHeaderSize_ = sizeof(BufferHeader);
		static const size_t kMaxDataSizeOneBuffer_ = 1472 - kBufferHeaderSize_;
		void InitHeader() {
			if (data_ == nullptr) {
				data_ = new uint8_t[kBufferHeaderSize_];
		   	}
			memcpy(data_, &header_, sizeof(header_));
		}
	public:
		static size_t GetMaxSizeOfNotSplit() { return kMaxDataSizeOneBuffer_; }
		static std::unique_ptr<Buffer> MakeBuffer() { return std::make_unique<Buffer> (); }
		Buffer(const uint8_t* buf = nullptr, size_t size = kBufferHeaderSize_);
		~Buffer();
		virtual void SetData(uint8_t*const buf, size_t size);
		//return buffer data,  buffer header + user data
		virtual uint8_t* GetBufferData();
		virtual size_t GetBufferSize();
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
	public:
		enum Stat {
			SUCCEE = 0,
			COMPLETING	= 1,
			COMPLETED	= 2,
			OUTTIME		= 3
		};
	private:
		std::size_t size_;
		std::unique_ptr<uint8_t[]> data_;
		Stat stat_ = COMPLETING;
	public:
		static size_t GetMaxSizeOfNotSplit() { return Buffer::GetMaxSizeOfNotSplit(); }
		uint8_t*const  GetData();
		void SetOnSendendCallback();
		Package() {
		}
		~Package() {}
		void SetData(std::unique_ptr<uint8_t[]> data, size_t size);
		void SetData(const char* buf, size_t size);
		size_t GetSize();
		operator bool() {
			return stat_ == COMPLETED;
		}
		Stat GetStat() { return stat_; }
		void SetOutTime() { stat_ = OUTTIME; }
		size_t GetBufferCount() {
			size_t cnt = 0;
			cnt = size_ / Buffer::GetMaxSizeOfNotSplit();
			if (cnt * Buffer::GetMaxSizeOfNotSplit() < size_) ++cnt;
			return cnt;
		}
	};

	//class BufferFromPackage : public Buffer
	//{
	//public:
	//	BufferFromPackage (std::shared_ptr<Package> package) :
	//	package_(package)	{
	//	}
	//	virtual ~BufferFromPackage () { 
	//		data_ = nullptr;
	//	}

	//	void SetData(uint8_t*const buf = 0, size_t size = 0) override;
	//	virtual uint8_t* GetBufferData() override;


	//private:
	//	std::shared_ptr<Package> package_;
	//};
}



#endif
