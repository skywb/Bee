#include "net/UDPSender.h"
#include "net/UDPReceiver.h"
#include "package/Package.h"

#include <iostream>
#include <boost/asio.hpp>


void ReceiveHandler(std::unique_ptr<Bee::Buffer> buf) {
	std::cout << buf->GetData() << std::endl;
}

int main(int argc, char *argv[]) {
	boost::asio::io_service service;
	boost::asio::ip::udp::endpoint local(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
	boost::asio::ip::udp::socket socket(service, local);
	std::thread th([&](){
			boost::asio::io_service::work work(service);
			service.run();
			});
	Bee::UDPReceiver receiver(socket, ReceiveHandler);
	receiver.AsyncReceive();
//	Bee::UDPEndPoint endpoint;
//	endpoint.IP = "127.0.0.1";
//	endpoint.port = 9999;
//	Bee::UDPSender sender(socket);
//	sender.AddClient(endpoint);
//
//	char msg[100] = "hello world";
//	auto buf = std::make_shared<Bee::Buffer> ((uint8_t*)msg, 12);
//	sender.SendBuffer(buf);
	std::this_thread::sleep_for(std::chrono::seconds(10));
	service.stop();
	th.join();
	return 0;
}
