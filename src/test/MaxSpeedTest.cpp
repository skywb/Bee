
#include <iostream>
#include <boost/asio.hpp>
#include "SpeedTest.hpp"

const std::string sender_ip = "192.168.1.105";
const std::string receiver_ip = sender_ip;
const short sender_port = 8888;
const short receiver_port =9999;



int main(int argc, const char *argv[])
{
	if (argc < 2) {
		std::cout << "please input [sender|receiver]" << std::endl;
		return 1;
	}
	boost::asio::io_service service;
	boost::asio::ip::udp::socket sock(service, boost::asio::ip::udp::v4());
	std::thread service_run([&](){
			boost::asio::io_service::work work(service);
			service.run();
			std::cout << "stop" << std::endl;
		});
	if (memcmp(argv[1], "receiver", 4) == 0) {
		boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(receiver_ip), receiver_port);
		boost::system::error_code error;
		//sock.set_option(boost::asio::ip::udp::socket::receive_buffer_size(1024*3000));
		sock.bind(endpoint, error);
		endpoint.port(sender_port);
		if (error) {
			std::cout << error.message() << std::endl;
			return 0;
		}
		char buf[1600];
		SpeedCalculate speed;
		while (true) {
			auto re = sock.receive_from(boost::asio::buffer(buf, 1500), endpoint);	
			speed.AddSize(re);
		}
	} else {
		boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address::from_string(sender_ip), sender_port);
		boost::system::error_code error;
		sock.bind(endpoint, error);
		endpoint.port(receiver_port);
		if (error) {
			std::cout << "bind error: " <<  error.message() << std::endl;
			return 0;
		}
		SpeedCalculate speed;
		char buf[1600];
		while (true) {
			auto re = sock.send_to(boost::asio::buffer(buf, 1500), endpoint);	
			speed.AddSize(re);
		}
	}

	
	return 0;
}
