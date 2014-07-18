/*
 * ClientThread.h
 *
 *  Created on: Jan 21, 2013
 *      Author: zhangbo
 */

#ifndef CLIENTTHREAD_H_
#define CLIENTTHREAD_H_

#include "Runnable.h"
#include <netinet/in.h>
#include "Properties.h"

#define BUFSIZE (6 * 1024)

class ClientThread: public Runnable {
protected:
	int socket;
	string type;
	bool needScreen;
	string ptyType;
	struct in_addr clientAddress;
	Properties* properties;

	int OpenPty(char* ttyName, char* clientIp, int screenNum);
	int OpenPtmx(char* ttyName, char* clientIp, int screenNum);

public:
	ClientThread();
	virtual ~ClientThread();

	virtual void Run();

	void SetPtyType(const string& pty);
	void SetNeedScreen(bool need);
	void SetType(const string& type);
	void SetClientAddress(struct in_addr address);
	void SetTtyConfig(Properties* prop);
	void SetClientSocket(int socket);
};

#endif /* CLIENTTHREAD_H_ */
