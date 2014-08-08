#ifndef RUNNABLE_H_
#define RUNNABLE_H_

class Runnable {
public:
	Runnable(void);
	virtual ~Runnable(void);

	virtual void Run() = 0;
};

#endif
