#ifndef PACKAGE_H
#define PACKAGE_H

#include <memory>

namespace Bee {
	class Package {
	private:
		std::size_t size_;
		std::unique_ptr<uint8_t> data_;

	public:
		std::unique_ptr<void*> GetData();

		void SetData(std::unique_ptr<void*> data, size_t size);

		size_t GetSize();
	};


	class Buffer {
	private:
		std::unique_ptr<uint8_t*> data_;
		size_t size_;
		int attribute;
		size_t pack_num_;
		size_t begin_;
		size_t count_;

	public:
		Buffer(uint8_t* buf, size_t size);
		uint8_t GetBufferData();

		uint8_t GetData();

		size_t Size();

		void SetData(uint8_t data, size_t size);
	};
}



#endif
