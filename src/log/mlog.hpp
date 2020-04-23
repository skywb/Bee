#pragma once 

#include <sstream>
#include <iostream>
#include <functional>
#include <string>
#include "mlog_def.hpp"

/*
** 用于在头文件内生成全局唯一对象
*/
template<typename T>
class GlobalVar {
public:
    static T VAR;
};
 
template<typename T> T GlobalVar<T>::VAR = nullptr;
 
/*
** 日志级别
*/
#define MLOG_DEBUG  0
#define MLOG_INFO   1
#define MLOG_WARN   2
#define MLOG_ERROR  3
#define MLOG_FATAL  4
 
/*
** mlog
*/
namespace mlog
{
    class LogMessage;
 
    /*
    ** 日志回调设置函数
    */
    static void SetMlogCallBack(MLogCallBack func) { GlobalVar<MLogCallBack>::VAR = func; }
}
 
/*
** 日志回调生成类
*/
class mlog::LogMessage
{
public:
    LogMessage(const char* file, int line, const char* func, int severity, MLogCallBack callback)
        : _file(file)
        , _line(line)
        , _func(func)
        , _severity(severity)
        , _callback(callback)
    {
    }
 
    ~LogMessage()
    {
        if (_callback)
        {
            std::string content = _stream.str();
            _callback(_file.c_str(), _line, _func.c_str(), _severity, content.c_str());
        }
    }
 
    std::ostringstream &stream() { return _stream; }
 
private:
    std::string _file;
    int _line;
    std::string _func;
    int _severity;
 
    MLogCallBack _callback;
    std::ostringstream _stream;
};
 
/*
** 实际使用宏
*/
#define LOG_DEBUG mlog::LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_DEBUG, GlobalVar<MLogCallBack>::VAR).stream()
#define LOG_INFO  mlog::LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_INFO,  GlobalVar<MLogCallBack>::VAR).stream()
#define LOG_WARN  mlog::LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_WARN,  GlobalVar<MLogCallBack>::VAR).stream()
#define LOG_ERROR mlog::LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_ERROR, GlobalVar<MLogCallBack>::VAR).stream()
#define LOG_FATAL mlog::LogMessage(__FILE__, __LINE__, __FUNCTION__, MLOG_FATAL, GlobalVar<MLogCallBack>::VAR).stream()
