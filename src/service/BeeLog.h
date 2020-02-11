#ifndef BEELOG_H_NO3AZRTA
#define BEELOG_H_NO3AZRTA

namespace Bee {

/*! \class BeeLog
*  \brief Brief class description
*
*  Detailed description
*/
class BeeLog
{
public:

	/*
	 * ** 日志回调函数原型
	 * ** @file 日志所在文件
	 * ** @line 日志所在代码行
	 * ** @func 日志所在函数
	 * ** @severity 日志级别
	 * ** @context  日志内容
	 * */
	typedef void(*BeeLogCallBack)(const char *file, int line, const char *func, int severity, const char *content);

	virtual ~BeeLog() { }

	void SetBeeLogCallback(BeeLogCallBack log_callback) {
		callback_ = log_callback;
	}
	static void DefaultCallback(const char *file, int line, const char *func, int severity, const char *content) { }
private:
	static BeeLogCallBack callback_;
	BeeLog() {}
};

}



#endif /* end of include guard: BEELOG_H_NO3AZRTA */
