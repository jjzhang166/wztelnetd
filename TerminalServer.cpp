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
	this->needScreen = true;
	this->local = true;
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

void TerminalServer::SetTtyMapFile(bool local, const char* file) {
	strcpy(ttyMapFile, file);
	this->local = local;
}

void childexit(int sig) {
	waitpid(-1, NULL, WNOHANG);
}

void segvexit(int sig) {
	exit(0);
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
	fcntl(sockfd, F_SETFD, FD_CLOEXEC);

	if (listen(sockfd, BACKLOG) == -1) {
		return;
	}

	::signal(SIGCHLD, &childexit);

	fflush(NULL);
	while (1) {
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size))
				== -1) {
			continue;
		}
		fcntl(new_fd, F_SETFD, FD_CLOEXEC);

		pid_t pid = fork();
		if (pid > 0) {
			close(new_fd);
			continue;
		} else if (pid == 0) {
			::signal(SIGPIPE, SIG_IGN);
			::signal(SIGCHLD, SIG_IGN);
			::signal(SIGSEGV, &segvexit);
			close(sockfd);
			ClientThread client;
			client.SetPtyType(this->ptyType);
			client.SetType(this->type);
			client.SetNeedScreen(this->needScreen);
			client.SetClientSocket(new_fd);
			client.SetClientAddress(their_addr.sin_addr);
			client.SetTtyMapFile(local, this->ttyMapFile);
			client.Run();
			exit(0);
		} else {
			continue;
		}
	}
}
