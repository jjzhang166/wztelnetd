/*
 * TerminalServer.h
 *
 *  Created on: Jan 18, 2013
 *      Author: zhangbo
 */

#ifndef TERMINALSERVER_H_
#define TERMINALSERVER_H_

#define thread_num 100

#include "Runnable.h"
#include <string>
#include "Properties.h"
using namespace std;

class TerminalServer: public Runnable {
protected:
	virtual void Run();
	int port;
	int count;
	string type;
	Properties* properties;
	bool needScreen;
	string ptyType;

public:
	TerminalServer();
	virtual ~TerminalServer();

	void SetNeedScreen(bool need);
	void SetPtyType(const string& pty);
	void SetPort(int port);
	void SetCount(int count);
	void SetType(const string& type);
	void SetTtyConfig(Properties* prop);
};

#endif /* TERMINALSERVER_H_ */
