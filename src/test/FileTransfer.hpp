#pragma once

#include "client/Bee.h"
#include <fstream>

class FileTransferTest : public Bee::PackageArrivedCallback {
public:
	FileTransferTest () = default;
	virtual ~FileTransferTest () = default;
	virtual void OnCallback(std::unique_ptr<Bee::Package> package) override;
private:
};

struct FileINFO {
	size_t file_name_len;
	size_t file_data_len;
};

void FileTransferTest::OnCallback(std::unique_ptr<Bee::Package> package) {
	FileINFO file;
	auto buf = package->GetData();
	memcpy(&file, buf, sizeof(file));
	char filename[1024];
	buf += sizeof(file);
	memcpy(filename, buf, file.file_name_len);
	filename[file.file_name_len] = 0;
	buf += file.file_name_len;
	std::ofstream file_stream;
	file_stream.open(filename, std::ios::binary | std::ios::out);
	if (!file_stream) {
		LOG_ERROR << "fopen error : " ;
	}
	file_stream.seekp(0, std::ios::beg);
	file_stream.write((char*)buf, file.file_data_len);
	file_stream.close();
	LOG_INFO << "receive a file : " << filename << " file len is " << file.file_data_len;
	std::cout << "receive a file : " << filename << " file len is " << file.file_data_len << std::endl;
}

