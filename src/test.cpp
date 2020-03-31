#include <iostream>
#include <boost/asio.hpp>

const std::string sender_ip = "192.168.1.105";
const std::string receiver_ip = sender_ip;
const short sender_port = 9999;
const short receiver_port =8888;

std::mutex mutex;

void CalculateSpeed(unsigned long long& pack_cnt) {
	unsigned long long pre_cnt = pack_cnt;
	auto pre_time = std::chrono::system_clock::now();
	while (true) {
		std::unique_lock<std::mutex> lock(mutex);
		unsigned long long now_cnt = pack_cnt;
		if (pre_cnt > 1e7) {
			pack_cnt -= pre_cnt;
			now_cnt = pack_cnt;
			pre_cnt -= pre_cnt;
		}
		lock.unlock();
		auto now_time = std::chrono::system_clock::now();
		auto dt = std::chrono::duration_cast<std::chrono::milliseconds> (now_time - pre_time);	
		if (dt.count() == 0) continue;
		std::cout << (now_cnt - pre_cnt) / dt.count() / 1024 << "MB/s" << std::endl;
		pre_time = now_time;
		pre_cnt = now_cnt;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	return ;
}



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
		//sock.connect(endpoint);
		if (error) {
			std::cout << error.message() << std::endl;
			return 0;
		}
		char buf[1600];
		unsigned long long cnt = 0;
		std::thread th(CalculateSpeed, std::ref(cnt));
		int cnt_t = 0;
		while (true) {
			auto re = sock.receive_from(boost::asio::buffer(buf, 1500), endpoint);	
			//auto re = sock.receive(boost::asio::buffer(buf, 1500));	
			if (re != 1500) std::cout << "---------------- " <<  re << std::endl;
			++cnt_t;
			//auto re = sock.receive_from(boost::asio::buffer(buf, 1500), endpoint, boost::asio::ip::udp::socket::message_end_of_record, error);
			if (error) {
				std::cout << "receive error: " << error.message() << std::endl;
			}	
			//std::cout << re << std::endl;
			std::unique_lock<std::mutex> lock(mutex);
			cnt += re;
			lock.unlock();
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
		char buf[1600];
		unsigned long long cnt = 0;
		std::thread th(CalculateSpeed, std::ref(cnt));
		while (true) {
			auto re = sock.send_to(boost::asio::buffer(buf, 1500), endpoint);	
			std::unique_lock<std::mutex> lock(mutex);
			cnt += re;
			lock.unlock();
		}
	}

	
	return 0;
}
