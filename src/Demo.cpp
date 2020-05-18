#include "client/Bee.h"

#include <iostream>
#include <mutex>
#include <fstream>
#include <cstring>

#include "test/FileTransfer.hpp"
#include "test/SpeedTest.hpp"

#ifdef WIN
#include <Windows.h>
#elif LINUX or UNIX
#endif



const std::string MULTYCAST_IP = "224.0.0.122";
const short MULTYCAST_PORT = 8999;
#ifdef WIN
const std::string LOCAL_IP = "192.168.1.105";
#elif LINUX or UNIX
const std::string LOCAL_IP = "172.28.78.68";
#endif
const std::string UNICAST_IP = LOCAL_IP;
const short UNICAST_PORT = 8888;

void Sender() {
	Bee::Connecter connecter;
	connecter.SetLocalIPAndPort("0.0.0.0", UNICAST_PORT);
	LOG_INFO << "Unicast is " << UNICAST_IP << " : " << UNICAST_PORT;
	//connecter.AddClient(MULTYCAST_IP, MULTYCAST_PORT);
	//LOG_INFO << "Multicast is " << MULTYCAST_IP << " : " << MULTYCAST_PORT;
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
	//SetMyLibraryLogCallback(LogCallback);
	Bee::Connecter connecter;
	const short local_port = port == 6999 ? 6998 : 6999;
	connecter.SetLocalIPAndPort(LOCAL_IP, local_port);
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

void ReceiverSpeedTest(const std::string IP, const short port) {
	Bee::Connecter connecter;
	const short local_port = port == 6999 ? 6998 : 6999;
	connecter.SetLocalIPAndPort(LOCAL_IP, local_port);
	connecter.AddService(IP, port);
	connecter.SetThreadCount(2);
	connecter.SetPackageArrivedCallback(std::make_unique<SpeedTest>());
#ifdef WIN
	LOG_INFO << "current system is windows";
	system("pause");
#elif LINUX or UNIX
	pause();
#endif
}


int cnt = 1;
std::mutex lock;

void SendData(Bee::Connecter& connecter,
	   	std::shared_ptr<SpeedCalculate> speed, const Bee::Error error, int number) {
	if (error) {
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		LOG_INFO << "not found client";
	}
	else {
		if (speed)
			speed->AddSize(Bee::Package::GetMaxSizeOfNotSplit());
	}
	//std::this_thread::sleep_for(std::chrono::microseconds(5));
	static char buf[16000];
	//lock.lock();
	//int cur = cnt;
	//++cnt;
	//lock.unlock();
	//memcpy(buf, &cur, 4);
	auto package = std::make_unique<Bee::Package> ();
	package->SetData(buf, package->GetMaxSizeOfNotSplit()*10);
	connecter.SendPackage(std::move(package),
		   	std::bind(SendData, std::ref(connecter), speed, std::placeholders::_1, 0));
}

void DataSender() {
	Bee::Connecter connecter;
	connecter.SetThreadCount(2);
	connecter.SetLocalIPAndPort(UNICAST_IP, UNICAST_PORT);
	LOG_INFO << "Unicast is " << UNICAST_IP << " : " << UNICAST_PORT;
	//std::cout << "Unicast is " << UNICAST_IP << " : " << UNICAST_PORT << std::endl;
	//connecter.AddClient(MULTYCAST_IP, MULTYCAST_PORT);
	connecter.AddClient("192.168.1.105", 9999);
	//LOG_INFO << "Multicast is " << MULTYCAST_IP << " : " << MULTYCAST_PORT;
	SendData(connecter, nullptr, Bee::Error::kSusseed, 0);
	SendData(connecter, nullptr, Bee::Error::kSusseed, 0);
#ifdef WIN
	LOG_INFO << "current system is windows";
	system("pause");
#elif LINUX or UNIX
	pause();
#endif
}

int main(int argc, const char *argv[]) {
	if (argc < 2) {
		std::cout << "Format is : Demo [sender|receiver]" << std::endl;
		return 1;
	}
	google::InitGoogleLogging(argv[0]);
	FLAGS_log_dir = "./";
	FLAGS_alsologtostderr = false;
	//FLAGS_minloglevel = 3;
	//google::ShutdownGoogleLogging();
//#ifdef LINUX
//	system("pwd");
//#elif WIN
//	system("chdir");
//#endif
	if (memcmp(argv[1], "receiver", 6) == 0) {
		std::string IP;
		short port;
		if (argc < 4) {
			std::cout << "please input ip and port: ";
			std::cin >> IP >> port;
		} else {
			IP = argv[2];
			port = atoi(argv[3]);
		}
		//Receiver(IP, port);
		ReceiverSpeedTest(IP, port);
	} else if (memcmp(argv[1], "sender", 6) == 0) {
		//Sender();
		DataSender();
	} else {
		std::cout << "Format is : Demo [sender|receiver]" << std::endl;
		return 1;
	}
	LOG_INFO << "exit";
	return 0;
}
