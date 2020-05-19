#pragma once


#include "client/Bee.h"
#include <fstream>


class SendBase
{
public:
	SendBase (const std::string local_IP, const short local_port) :
		IP_(local_IP), port_(local_port) {}
	virtual ~SendBase ();
	virtual void Send() = 0;
protected:
	const std::string IP_;
	const short port_;
};

class ReceiveBase
{
public:
	ReceiveBase (const std::string local_IP, const short local_port) :
		IP_(local_IP), port_(local_port) {}
	virtual ~ReceiveBase ();
	virtual void Receive(const std::string IP, const short port) = 0;
protected:
	const std::string IP_;
	const short port_;
};

