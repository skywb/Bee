#include "Service.h"

using namespace Bee;

Service::Service() {
	package_control_ = std::make_unique<PackageControl> ();
	recover_manager_ = std::make_unique<RecoverManager> ();
	receiver_ = std::make_unique<UDPReceiver> ();
	sender_ = std::make_unique<UDPSender> ();
}

// Only for Unittest
Service::Service(std::unique_ptr<PackageControl> package_control, 
		std::unique_ptr<RecoverManager> recover_manager,
		std::unique_ptr<UDPReceiver> receiver,
		std::unique_ptr<UDPSender> sender) {
	if (package_control)
		package_control_ = std::move(package_control);
	else 
		package_control_ = std::make_unique<PackageControl> ();

	if (recover_manager)
		recover_manager_ = std::move(recover_manager);
	else 
		recover_manager_ = std::make_unique<RecoverManager> ();

	if (receiver)
		receiver_ = std::move(receiver);
	else 
		receiver_ = std::make_unique<UDPReceiver> ();

	if (sender)
		sender_ = std::move(sender);
	else 
		sender_ = std::make_unique<UDPSender> ();
}


void Service::Init(int thread_count) {
	for (int i = 0; i < thread_count; ++i) {
		threads_.emplace_back([&]() {
				boost::asio::io_service::work work(service_);
				service_.run();
				});
	}
}

void Service::SendPackage(std::unique_ptr<Package> package) {
	auto buffers = std::move(package_control_->SplitPackage(std::move(package)));
	for (int i=0; i<buffers.size(); ++i) {
		auto buffer = std::make_shared<Buffer> (buffers[i].release());
		sender_->SendBuffer(buffer);
		recover_manager_->AddPackRecord(buffer);
	}
}


void Service::ReceivedHandler(std::unique_ptr<Buffer> buffer) {
	// TODO - implement Service::ReceivedHandler
	throw "Not yet implemented";
}

void Service::GetRTT() {
	// TODO - implement Service::GetRTT
	throw "Not yet implemented";
}
