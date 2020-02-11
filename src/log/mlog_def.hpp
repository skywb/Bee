//mlog_def.hpp
#pragma once
 
/*
** 日志回调函数原型
** @file 日志所在文件
** @line 日志所在代码行
** @func 日志所在函数
** @severity 日志级别
** @context  日志内容
*/
typedef void(*MLogCallBack)(const char *file, int line, const char *func, int severity, const char *content);