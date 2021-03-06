#include "Connecter.h"

using namespace  Bee;

Connecter::Connecter() {
	service_ = std::make_unique<Service>();
	service_->Init();
}

Connecter::~Connecter() {
	service_->Stop();
}
void Connecter::SetLocalIPAndPort(const std::string IP, const short port) {
	service_->SetLocalAddress(IP,port);
	service_->SetThreadCount(thread_count_);
	service_->Run();
}
void Connecter::SetThreadCount(const size_t thread_count) {
	thread_count_ = thread_count;
}

void Connecter::AddService(const std::string IP, const short port) {
	service_->ConnectService(IP, port);
}
void Connecter::RemoveService(const std::string IP, const short port) {
	service_->DeConnectService(IP, port);
}

void Connecter::AddClient(const std::string IP, const short port) {
	service_->AddClient(IP, port);
}

void Connecter::RmoveClient(const std::string IP, const short port) {
	service_->RemoveClient(IP, port);
}

void Connecter::SendPackage(std::unique_ptr<Package> package, BeeCallback callback) {
	service_->SendPackage(std::move(package), callback);
}

bool Connecter::RequestToServeice(std::unique_ptr<Package> package) {
	return service_->Request(std::move(package));
}

bool Connecter::RequestToServeice(std::unique_ptr<Package> package,
	   	const std::string IP, const short port) {
	return service_->Request(std::move(package), UDPEndPoint{IP,port});
}

void Connecter::SetPackageArrivedCallback(Callback callback) {
	/*! TODO: Todo description here
	*  \todo Todo description here
	*/
	//service_->SetPackageArrivedCallback(callback);
}

void Connecter::SetPackageArrivedCallback(std::unique_ptr<PackageArrivedCallback> callback) {
	service_->SetPackageArrivedCallback(std::move(callback));
}

void Connecter::SetPackageSendedCallback(Callback callback) {
	/*! TODO: Todo description here
	*  \todo Todo description here
	*/
}

void Connecter::SetPackageSendedCallback(std::unique_ptr<PackageSendedCallback> callback) {
	/*! TODO: Todo description here
	*  \todo Todo description here
	*/
}


// set sender heater
void Connecter::SetHeartRate(const std::size_t ms) {
	service_->SetHeartbeatRate(ms);
}

size_t Connecter::GetRTT() {
	return service_->GetRTT();
}

void Connecter::SetTranspond(bool transpond) {
	service_->SetTranspond(transpond);
}

void Connecter::SetBufferOutTime(int ms) {
	// TODO - implement Connecter::operation
	throw "Not yet implemented";
}

int Connecter::GetBufferOutTime() {
	// TODO - implement Connecter::operation
	throw "Not yet implemented";
}


void Connecter::SetBufferHistorySize(const size_t size) {
	service_->SetBufferMaxCount(size);
}
