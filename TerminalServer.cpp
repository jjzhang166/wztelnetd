/*
 * TerminalServer.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: zhangbo
 */

#include "TerminalServer.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "ThreadCreator.h"
#include "ClientThread.h"
#include <string.h>
#include "Messages.h"

#define BACKLOG 100

pthread_mutex_t mainMutex;

pthread_cond_t mainCond;

TerminalServer::TerminalServer() {
	this->port = 3231;
	this->type = "vt100";
	this->count = 256;
	//this->properties = NULL;
	this->needScreen = true;
}

TerminalServer::~TerminalServer() {
}

void TerminalServer::SetPort(int port) {
	this->port = port;
}

void TerminalServer::SetCount(int count) {
	this->count = count;
}

void TerminalServer::SetType(const string& type) {
	this->type = type;
}

void TerminalServer::SetPtyType(const string& pty) {
	this->ptyType = pty;
}

void TerminalServer::SetNeedScreen(bool need) {
	this->needScreen = need;
}

int close_on_exec_on(int fd) {
	return fcntl(fd, F_SETFD, FD_CLOEXEC);
}

void TerminalServer::SetTtyMapFile(bool local, const char* file) {
	strcpy(ttyMapFile, file);
	this->local = local;
}

void TerminalServer::Run() {
	int sockfd = 0, new_fd = 0;
	int yes = 1;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	socklen_t sin_size = 0;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return;
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		return;
	}

	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		printf(ERROR_CAN_NOT_OPEN_PORT, port);
		return;
	}
	close_on_exec_on(sockfd);

	if (listen(sockfd, BACKLOG) == -1) {
		return;
	}

	while (1) {
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size))
				== -1) {
			continue;
		}
		close_on_exec_on(new_fd);

		ClientThread *client = new ClientThread();
		client->SetPtyType(this->ptyType);
		client->SetNeedScreen(this->needScreen);
		client->SetClientSocket(new_fd);
		client->SetClientAddress(their_addr.sin_addr);
		client->SetTtyMapFile(local, this->ttyMapFile);
		ThreadCreator::StartThread(client);
	}
}
