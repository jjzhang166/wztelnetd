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
	int clientSocket;
	string type;
	bool needScreen;
	string ptyType;
	struct in_addr clientAddress;
	char ttyMapFile[1024];
	bool local;

	int OpenPty(char* ttyName, char* clientIp, char* screenNum);
	int OpenPtmx(char* ttyName, char* clientIp, char* screenNum);

public:
	ClientThread();
	virtual ~ClientThread();

	virtual void Run();

	void SetPtyType(const string& pty);
	void SetNeedScreen(bool need);
	void SetType(const string& type);
	void SetClientAddress(struct in_addr address);
	void SetTtyMapFile(bool local, const char* file);
	void SetClientSocket(int socket);
private:
	string FindTty(const string& name);
};

#endif /* CLIENTTHREAD_H_ */
