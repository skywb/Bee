#pragma once
 
#ifdef BEE_LIBARY_API
#define BEE_LIBARY_API __declspec(dllexport)
#else
#define BEE_LIBARY_API __declspec(dllimport)
#endif
 

#include "glog/logging.h"
#include "log/mlog.hpp"
#include "client/Connecter.h"

 
//BEE_LIBARY_API void SetMyLibraryLogCallback(MLogCallBack logCallback);
//void SetMyLibraryLogCallback(MLogCallBack logCallback);
