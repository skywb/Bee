#include "Test.h"
#include "FileTransfer.hpp"

class FileSend : public SendBase {
public:
	FileSend ();
	virtual ~FileSend ();
	void Send() override;
private:
	/* data */
};

void FileSend::Send() {
	const std::string MULTYCAST_IP = "224.0.0.1";
	const short MULTYCAST_PORT = 8999;
	Bee::Connecter connecter;
	connecter.SetLocalIPAndPort("0.0.0.0", port_);
	LOG_INFO << "Unicast is " << IP_ << " : " << port_;
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

class FileReceive : public ReceiveBase {
public:
	FileReceive (const std::string local_IP, const short local_port);
	virtual ~FileReceive ();
	void Receive(const std::string IP, const short port) override;
private:
};

void FileReceive::Receive(const std::string IP, const short port) {
	Bee::Connecter connecter;
	connecter.SetLocalIPAndPort(IP_, port_);
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

