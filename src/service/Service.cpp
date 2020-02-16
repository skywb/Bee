#include "Service.h"
#include "service/mlog.h"

using namespace Bee;

Service::Service() :
	socket_(service_) {

	package_control_ = std::make_unique<PackageControl> ();
	recover_manager_ = std::make_unique<RecoverManager> (service_, [&](std::shared_ptr<Buffer> buf, UDPEndPoint endpoint) {
			sender_->SendBufferTo(buf, endpoint);
			});

	receiver_ = std::make_unique<UDPReceiver> (socket_,
		   	std::bind(&Service::ReceivedHandler, this, std::placeholders::_1, std::placeholders::_2));

	sender_ = std::make_unique<UDPSender> (socket_);
}

// Only for Unittest
Service::Service(std::unique_ptr<PackageControl> package_control, 
		std::unique_ptr<RecoverManager> recover_manager,
		std::unique_ptr<UDPReceiver> receiver,
		std::unique_ptr<UDPSender> sender) : socket_(service_) {
	if (package_control)
		package_control_ = std::move(package_control);
	else 
		package_control_ = std::make_unique<PackageControl> ();

	if (recover_manager)
		recover_manager_ = std::move(recover_manager);
	else 
		recover_manager_ = std::make_unique<RecoverManager> (service_, [&](std::shared_ptr<Buffer> buf, UDPEndPoint endpoint) {
				sender_->SendBufferTo(buf, endpoint);
				});

	if (receiver)
		receiver_ = std::move(receiver);
	else 
		receiver_ = std::make_unique<UDPReceiver> (socket_,
				std::bind(&Service::ReceivedHandler, this, std::placeholders::_1, std::placeholders::_2));

	if (sender)
		sender_ = std::move(sender);
	else 
		sender_ = std::make_unique<UDPSender> (socket_);
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
		std::shared_ptr<Buffer> buffer(buffers[i].release());
		sender_->SendBuffer(buffer);
		recover_manager_->AddPackRecord(buffer);
	}
}


void Service::ReceivedHandler(std::unique_ptr<Buffer> buffer, UDPEndPoint endpoint) {
	switch (buffer->GetBufferHeader().type) {
		case BufferType::DATA:
			OnDataRecived(std::move(buffer));
			break;
		case BufferType::NACK:
			OnNACKRecived(std::move(buffer), endpoint);
			break;
		case BufferType::BUFFER_NOT_FOUND:
			OnNotFoundPackRecived(std::move(buffer));
			break;
		default:
			LOG_ERROR << "not found this type";
	}
}

void Service::GetRTT() {
	// TODO - implement Service::GetRTT
	throw "Not yet implemented";
}

void Service::OnDataRecived(std::unique_ptr<Buffer> buf) {
	recover_manager_->PackageArrived(buf->GetBufferHeader().pack_num);
	package_control_->OnReceivedBuffer(std::move(buf));
}

void Service::OnNACKRecived(std::unique_ptr<Buffer> buf, UDPEndPoint endpoint) {
	recover_manager_->NACKReceived(buf->GetBufferHeader().pack_num, endpoint);
}

void Service::OnNotFoundPackRecived(std::unique_ptr<Buffer> buf) {
	package_control_->OnBufferNotFound(buf->GetBufferHeader().pack_num);
}

