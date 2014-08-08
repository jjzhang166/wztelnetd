#ifndef RUNNABLE_H_
#define RUNNABLE_H_

/**
 * 所有线程类的父类，线程类必须实现Run函数。
 */
class Runnable {
public:
	Runnable(void);
	virtual ~Runnable(void);

	virtual void Run() = 0;
};

#endif
