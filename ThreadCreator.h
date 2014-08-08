#ifndef THREADCREATER_H_
#define THREADCREATER_H_

#include "Runnable.h"
#include <pthread.h>

class ThreadCreator {
private:
	Runnable* runnable;
	bool autoDestroy;
	pthread_t pid;

public:
	ThreadCreator(Runnable* runnable);
	ThreadCreator(Runnable* runnable, bool autoDestroy);
	~ThreadCreator(void);

	int Start();

	unsigned long GetHandle();

	void Wait();

	Runnable* GetRunnable();
	bool AutoDestroy();

	static ThreadCreator* StartThread(Runnable* runnable);

};

#endif
