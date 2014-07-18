#pragma once

#include "Runnable.h"
#include <pthread.h>

class ThreadCreator {
private:
	Runnable* runnable;
	int handle;
	bool autoDestroy;
	pthread_t pid;

public:
	ThreadCreator(Runnable* runnable);
	ThreadCreator(Runnable* runnable, bool autoDestroy);
	~ThreadCreator(void);

	int Start();

	int GetHandle();

	void Wait();

	Runnable* GetRunnable();
	bool AutoDestroy();

	static ThreadCreator* StartThread(Runnable* runnable);

};

