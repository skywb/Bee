#pragma once 
#ifndef MLOG_HPP_9HIUCIRV
#define MLOG_HPP_9HIUCIRV




#include "glog/logging.h"
#define LOG_DEBUG LOG(INFO)
#define LOG_INFO  LOG(INFO)
#define LOG_WARN  LOG(WARNING)
#define LOG_ERROR LOG(ERROR)
#define LOG_FATAL LOG(FATAL)
#endif /* end of include guard: MLOG_HPP_9HIUCIRV */
