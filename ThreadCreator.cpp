#include "ThreadCreator.h"

ThreadCreator::ThreadCreator(Runnable* runnable) :
		pid(0) {
	this->handle = 0;
	this->runnable = runnable;
	this->autoDestroy = false;
}

ThreadCreator::ThreadCreator(Runnable* runnable, bool autoDestroy) :
		pid(0) {
	this->handle = 0;
	this->runnable = runnable;
	this->autoDestroy = autoDestroy;
}

ThreadCreator::~ThreadCreator(void) {

}

void* ThreadFunction(void* pthread) {
	ThreadCreator* creator = (ThreadCreator*) pthread;
	creator->GetRunnable()->Run();
	if (creator->AutoDestroy()) {
		delete creator->GetRunnable();
		delete creator;
	}
	return 0;
}

Runnable* ThreadCreator::GetRunnable() {
	return this->runnable;
}

bool ThreadCreator::AutoDestroy() {
	return this->autoDestroy;
}

int ThreadCreator::Start() {
	this->handle = pthread_create(&pid, (const pthread_attr_t*) 0,
			ThreadFunction, (void*) this);
	return this->handle;
}

int ThreadCreator::GetHandle() {
	return this->handle;
}

ThreadCreator* ThreadCreator::StartThread(Runnable* runnable) {
	ThreadCreator* tc = new ThreadCreator(runnable, true);
	tc->Start();
	return tc;
}

void ThreadCreator::Wait() {
	void* pvoid = 0;
	pthread_join(pid, &pvoid);
}
