#ifndef TESTBASE_H_MDE58JJS
#define TESTBASE_H_MDE58JJS

#include "client/Connecter.h"
#include <memory>

class TestBase
{
public:
	virtual void OnCallback(std::unique_ptr<Bee::Package> package) = 0;
private:
};


#endif /* end of include guard: TESTBASE_H_MDE58JJS */
