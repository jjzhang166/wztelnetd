#pragma once
class Runnable {
public:
	Runnable(void);
	virtual ~Runnable(void);

	virtual void Run() = 0;
};

