#include "client/Bee.h"
#include "client/Connecter.h"
#include "Service/mlog.h"

#include <iostream>
#include <mutex>
#include <fstream>
#include <cstring>

#ifdef WIN
#include <Windows.h>
#elif LINUX or UNIX
#endif



const std::string MULTYCAST_IP = "224.0.0.1";
const short MULTYCAST_PORT = 8999;
#ifdef WIN
const std::string LOCAL_IP = "192.168.1.105";
#elif LINUX or UNIX
const std::string LOCAL_IP = "172.20.119.100";
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


struct FileINFO {
	size_t file_name_len;
	size_t file_data_len;
};

int main(int argc, char *argv[]) {
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

	SetMyLibraryLogCallback(LogCallback);
	Bee::Connecter connecter;
	if (memcmp(argv[1], "receiver", 6) == 0) {
		connecter.SetLocalIPAndPort(LOCAL_IP, 6999);
		connecter.AddService(MULTYCAST_IP, MULTYCAST_PORT);
		//connecter.AddService(UNICAST_IP, UNICAST_PORT);
		connecter.SetPackageArrivedCallback([](std::unique_ptr<Bee::Package> package){
				FileINFO file;
				auto buf = package->GetData();
				memcpy(&file, buf, sizeof(file));
				char filename[1024];
				buf += sizeof(file);
				memcpy(filename, buf, file.file_name_len);
				filename[file.file_name_len] = 0;
				buf += file.file_name_len;
				//auto fp = fopen(filename, "w+");
				std::ofstream file_stream;
				file_stream.open(filename, std::ios::binary | std::ios::out);
				if (!file_stream) {
					LOG_ERROR << "fopen error : " ;
				}
				file_stream.seekp(0, std::ios::beg);
				file_stream.write((char*)buf, file.file_data_len);
				//fseek(fp, 0, SEEK_SET);
				//auto re = fwrite(buf, 1, file.file_data_len, fp);
				//fclose(fp);
				file_stream.close();
				LOG_INFO << "receive a file : " << filename << " file len is " << file.file_data_len;
			});
#ifdef WIN
		LOG_INFO << "current system is windows";
		system("pause");
#elif LINUX or UNIX
		pause();
#endif
	} else if (memcmp(argv[1], "sender", 6) == 0) {
		//connecter.SetLocalIPAndPort(LOCAL_IP, UNICAST_PORT);
		connecter.SetLocalIPAndPort("0.0.0.0", UNICAST_PORT);
		connecter.AddClient(MULTYCAST_IP, MULTYCAST_PORT);
		//connecter.AddClient(LOCAL_IP, 3888);
		//connecter.AddClient(UNICAST_IP, UNICAST_PORT);
		std::string msg;
		std::cout << "please input file path: ";
		connecter.SetHeartRate(5000);
		while (std::cin >> msg) {
			//auto fp = fopen(msg.c_str(), "r");
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
	} else {
		std::cout << "Format is : Demo [sender|receiver]" << std::endl;
		return 1;
	}
	LOG_INFO << "exit";
	return 0;
}
