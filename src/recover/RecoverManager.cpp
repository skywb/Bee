#include "RecoverManager.h"

using namespace Bee;
void RecoverManager::PackageArrived(size_t package_num) {
	// TODO - implement RecoverManager::PackageArrived
	throw "Not yet implemented";
}

void RecoverManager::AddPackRecord(size_t packnum) {
	// TODO - implement RecoverManager::AddPackRecord
	throw "Not yet implemented";
}

RecoverManager::RecoverManager(boost::asio::io_service& service)
	: service_(service),
	timer_pack_outtime_(service_) {
	// TODO - implement RecoverManager
	throw "Not yet implemented";
}

void RecoverManager::AddPackToHistroy(size_t package_num, std::shared_ptr<Buffer> pack_data) {
	// TODO - implement RecoverManager::AddPackToHistroy
	throw "Not yet implemented";
}

void RecoverManager::ClearOutTimeHistory() {
	// TODO - implement RecoverManager::ClearOutTimeHistory
	throw "Not yet implemented";
}

std::shared_ptr<Buffer> RecoverManager::NACKReceived(size_t pack_num) {
	// TODO - implement RecoverManager::NACKReceived
	throw "Not yet implemented";
}
