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
	int port;
	int count;
	string type;
	string clientType;
	char ttyMapFile[1024];
	bool needScreen;
	string ptyType;
	bool local; //使用实达（false）/汇金（true）配置

public:
	TerminalServer();
	virtual ~TerminalServer();

	virtual void Run();

	void SetNeedScreen(bool need);
	void SetPtyType(const string& pty);
	void SetPort(int port);
	void SetCount(int count);
	void SetType(const string& type);
	void SetClientType(const string& clientType);
	void SetTtyMapFile(bool local, const char* file);
};

#endif /* TERMINALSERVER_H_ */
