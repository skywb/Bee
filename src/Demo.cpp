#include "client/Bee.h"

#include <iostream>
#include <mutex>
#include <fstream>
#include <cstring>

#include "test/FileTransfer.hpp"

#ifdef WIN
#include <Windows.h>
#elif LINUX or UNIX
#endif



const std::string MULTYCAST_IP = "224.0.0.1";
const short MULTYCAST_PORT = 8999;
#ifdef WIN
const std::string LOCAL_IP = "192.168.1.105";
#elif LINUX or UNIX
const std::string LOCAL_IP = "172.26.192.41";
#endif
const std::string UNICAST_IP = LOCAL_IP;
const short UNICAST_PORT = 8888;

std::ofstream log_stream;

void LogCallback(const char *file, int line, const char *func, int severity, const char *content) {
	static std::mutex mutex;
	std::lock_guard<std::mutex> lock(mutex);
	switch(severity) {
		case MLOG_DEBUG:
			std::cout << "[DEBUG] ";
			break;
		case MLOG_INFO:
			std::cout << "[INFO] ";
			break;
		case MLOG_WARN:
			std::cout << "[WARN] ";
			break;
		case MLOG_ERROR:
			std::cout << "[ERROR] ";
			break;
		case MLOG_FATAL:
			std::cout << "[FATAL] ";
			break;
	}
	std::cout << file << ":" << line << ":" << func << ":  " << content << std::endl;
}

void Sender() {
	SetMyLibraryLogCallback(LogCallback);
	Bee::Connecter connecter;
	connecter.SetLocalIPAndPort("0.0.0.0", UNICAST_PORT);
	LOG_INFO << "Unicast is " << UNICAST_IP << " : " << UNICAST_PORT;
	connecter.AddClient(MULTYCAST_IP, MULTYCAST_PORT);
	LOG_INFO << "Multicast is " << MULTYCAST_IP << " : " << MULTYCAST_PORT;
	std::string msg;
	std::cout << "please input file path: ";
	connecter.SetHeartRate(5000);
	while (std::cin >> msg) {
		std::ifstream file_stream;
		file_stream.open(msg, std::ios::binary | std::ios::in);
		if (!file_stream) {
			LOG_ERROR << "fopen error ";
			continue;
		}
		file_stream.seekg(0, std::ios::end);
		auto size = file_stream.tellg();
		LOG_INFO << "file length is : " << size << " Bytes";
		file_stream.seekg(0, std::ios::beg);
		FileINFO file;
		file.file_name_len = msg.size();
		file.file_data_len = size;
		size_t buf_size = sizeof(FileINFO)+file.file_name_len+file.file_data_len;
		auto buf = std::make_unique<uint8_t[]> (buf_size);
		uint8_t* p = buf.get();
		memcpy(p, &file, sizeof(file));
		p+=sizeof(file);
		memcpy(p, msg.c_str(), file.file_name_len);
		p += file.file_name_len;
		if (!file_stream.read((char*)p, file.file_data_len)) {
			LOG_ERROR << "read error";
			continue;
		}
		auto package = std::make_unique<Bee::Package> ();
		package->SetData(std::move(buf), buf_size);
		//package->SetData(msg.c_str(), msg.size());
		auto pp = new uint8_t[60];
		connecter.SendPackage(std::move(package));
		file_stream.close();
		LOG_INFO << "Send file len " << file.file_data_len;
	}
}

void Receiver(const std::string IP, const short port) {
	SetMyLibraryLogCallback(LogCallback);
	Bee::Connecter connecter;
	connecter.SetLocalIPAndPort(LOCAL_IP, 6999);
	connecter.AddService(IP, port);
	//connecter.AddService(UNICAST_IP, UNICAST_PORT);
	connecter.SetPackageArrivedCallback(std::make_unique<FileTransferTest>());
#ifdef WIN
	LOG_INFO << "current system is windows";
	system("pause");
#elif LINUX or UNIX
	pause();
#endif

}

int main(int argc, const char *argv[])
{
	if (argc < 2) {
		std::cout << "Format is : Demo [sender|receiver]" << std::endl;
		return 1;
	}
	log_stream.open("log.txt");
#ifdef LINUX
	system("pwd");
#elif WIN
	system("chdir");
#endif
	if (memcmp(argv[1], "receiver", 6) == 0) {
		std::string IP;
		short port;
		if (argc < 4) {
			std::cout << "please input ip and port: ";
			std::cin >> IP >> port;
			Receiver(IP, port);
		} else {
			Receiver(argv[2], atoi(argv[3]));
		}
	} else if (memcmp(argv[1], "sender", 6) == 0) {
		Sender();
	} else {
		std::cout << "Format is : Demo [sender|receiver]" << std::endl;
		return 1;
	}
	LOG_INFO << "exit";
	return 0;
}
