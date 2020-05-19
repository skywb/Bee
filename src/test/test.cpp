#include <iostream>
#include <boost/asio.hpp>

const std::string sender_ip = "192.168.1.105";
const std::string receiver_ip = sender_ip;
const short sender_port = 8888;
const short receiver_port =3888;

class SpeedCalculate
{
public:
	SpeedCalculate () : 
		data_size_(0),
		begin_time_(std::chrono::system_clock::now()),
		switch_(false)	{
			Run();
	}
	virtual ~SpeedCalculate () {
		switch_ = true;
		th_.join();
	}

	void AddSize(const unsigned long long Bytes) {
		std::lock_guard<std::mutex> lock(mutex_);
		data_size_ += Bytes;
	}

private:
	void Callback() {
		while (true) {
			if (!switch_) return;
			std::unique_lock<std::mutex> lock(mutex_);
			auto now_time = std::chrono::system_clock::now();
			auto dt = std::chrono::duration_cast<std::chrono::milliseconds> (now_time - begin_time_);	
			if (dt.count() != 0) {
				std::cout << data_size_  / dt.count() / 1024 << "MB/s" << std::endl;
				begin_time_ = now_time;
				data_size_ = 0;
			}
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
	void Run() {
		switch_ = true;
		std::thread th(std::thread(std::bind(&SpeedCalculate::Callback, this)));
		th_.swap(th);
	}

	unsigned long long data_size_;
	std::mutex mutex_;
	std::chrono::time_point<std::chrono::system_clock> begin_time_;
	std::thread th_;
	bool switch_;
};




int fun(int argc, const char *argv[])
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
		SpeedCalculate speed;
		int cnt_t = 0;
		while (true) {
			auto re = sock.receive_from(boost::asio::buffer(buf, 1500), endpoint);	
			if (error) {
				std::cout << "receive error: " << error.message() << std::endl;
			}	
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
		char buf[1600];
		SpeedCalculate speed;
		int cnt = 0;
		auto beg = std::chrono::system_clock::now();
		auto end = std::chrono::system_clock::now();
		while (true) {
			//if (cnt % 100000 == 0) {
			//	end = std::chrono::system_clock::now();
			//	auto t = std::chrono::duration_cast<std::chrono::microseconds> (end- beg);
			//	std::cout << t.count() << std::endl;
			//	beg = end;
			//}	
			auto re = sock.send_to(boost::asio::buffer(buf, 1500), endpoint);	
			speed.AddSize(re);
		}
	}

	
	return 0;
}

const std::string Kmulticast_ip = "224.0.0.1";

int main(int argc, const char *argv[])
{
	fun(argc, argv);
	return 0;
}
