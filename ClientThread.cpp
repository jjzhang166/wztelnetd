/*
 * ClientThread.cpp
 *
 *  Created on: Jan 21, 2013
 *      Author: zhangbo
 */

#include "ClientThread.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <termios.h>
#include <arpa/telnet.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <arpa/telnet.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include "Messages.h"
#include <math.h>
#include "Util.h"

#define LOG(msg)
#define EXIT_MAIN_PROCESS(msg) break;
//#define LOG(msg) printf(msg); fflush(NULL);
//#define EXIT_MAIN_PROCESS(msg) printf(#msg ": count=%d; errno=%d\r\n", count, errno);	fflush(NULL); break;

ClientThread::ClientThread() {
	this->local = true;
	this->clientSocket = 0;
	this->needScreen = true;
	this->ptyType = "bsd";
}

ClientThread::~ClientThread() {
}

void ClientThread::SetPtyType(const string& pty) {
	this->ptyType = pty;
}

void ClientThread::SetNeedScreen(bool need) {
	this->needScreen = need;
}

void ClientThread::SetClientSocket(int socket) {
	this->clientSocket = socket;
}

void ClientThread::SetClientAddress(struct in_addr address) {
	this->clientAddress = address;
}

void ClientThread::SetTtyMapFile(bool local, const char* file) {
	strcpy(this->ttyMapFile, file);
	this->local = local;
}

string ClientThread::FindTty(const string& name) {
	Properties prop;
	if (local) {
		prop.Load(this->ttyMapFile);
	} else {
		prop.LoadTable(this->ttyMapFile);
	}
	return prop.GetString(name);
}

int ClientThread::OpenPtmx(char* ttyName, char* clientIp, char* screenNum) {
	char key[128];
	sprintf(key, "%s.%s", clientIp, screenNum);
	string tty = this->FindTty(key);
	if (tty == "") {
		printf(ERROR_CAN_NOT_FOUND_CFG, clientIp, screenNum);
		return -1;
	}

	int p = open("/dev/ptmx", O_RDWR | O_NOCTTY);
	if (p < 0) {
		return -1;
	}
	grantpt(p); /* change permission of slave */
	unlockpt(p); /* unlock slave */

	strcpy(ttyName, ptsname(p));
	return p;
}

int ClientThread::OpenPty(char* ttyName, char* clientIp, char* screenNum) {
	char key[128];
	sprintf(key, "%s.%s", clientIp, screenNum);
	string tty = this->FindTty(key);
	if (tty == "") {
		printf(ERROR_CAN_NOT_FOUND_CFG, clientIp, screenNum);
		return -1;
	}

	sprintf(ttyName, "/dev/%s", tty.c_str());
	ttyName[5] = 't';

	char ptyName[128];
	strcpy(ptyName, ttyName);
	ptyName[5] = 'p';

	struct stat stb;
	if (stat(ptyName, &stb) < 0) {
		return -1;
	}
	int p = open(ptyName, O_RDWR | O_NOCTTY);
	if (p >= 0) {
		return p;
	}
	return -1;
}

int ClientThread::ReadScreenNumber(int socket, char* screen) {
	if (!needScreen) {
		screen[0] = '\0';
		return 0;
	}

	char chr1[] = { 0xff, 0xf1, 0x18, 0x00, 0x00, 0x00 };
	char chr2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	socket_send(socket, chr1, 6);
	char num[12];
	int len = recv(socket, num, 12, 0) - 2;
	memcpy(screen, num + 2, len);
	screen[len] = 0;
	socket_send(socket, chr2, 6);
	return len;
}

ssize_t ClientThread::SafeReadPtyfd(int fd, void *buf, size_t count,
		int *retry) {
	ssize_t n;
	do {
		n = read(fd, buf, count);
		if (n < 0) {
			if (errno == EIO) {
				*retry = 1;
				break;
			}
			*retry = 0;
			if (errno == EINTR) {
				continue;
			}
			if (errno == EAGAIN) {
				continue;
			}
		}
		break;
	} while (true);
	return n;
}

ssize_t ClientThread::SafeReadSocket(int fd, void *buf, size_t count) {
	ssize_t n;
	do {
		n = read(fd, buf, count);
		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			if (errno == EAGAIN) {
				continue;
			}
			break;
		} else {
			return n;
		}
	} while (true);

	return n;
}

unsigned char *ClientThread::RemoveIacs(unsigned char *ts, int tsLen, int ttyFd,
		int *pnum_totty) {
	unsigned char *ptr0 = ts;
	unsigned char *ptr = ptr0;
	unsigned char *totty = ptr;
	unsigned char *end = ptr + tsLen;
	int num_totty;

	while (ptr < end) {
		if (*ptr != IAC) {
			char c = *ptr;

			*totty++ = c;
			ptr++;
			/* We map \r\n ==> \r for pragmatic reasons.
			 * Many client implementations send \r\n when
			 * the user hits the CarriageReturn key.
			 */
			if (c == '\r' && ptr < end && (*ptr == '\n' || *ptr == '\0'))
				ptr++;
			continue;
		}

		if ((ptr + 1) >= end)
			break;
		if (ptr[1] == NOP) { /* Ignore? (putty keepalive, etc.) */
			ptr += 2;
			continue;
		}
		if (ptr[1] == IAC) { /* Literal IAC? (emacs M-DEL) */
			*totty++ = ptr[1];
			ptr += 2;
			continue;
		}

		/*
		 * TELOPT_NAWS support!
		 */
		if ((ptr + 2) >= end) {
			/* Only the beginning of the IAC is in the
			 buffer we were asked to process, we can't
			 process this char */
			break;
		}
		/*
		 * IAC -> SB -> TELOPT_NAWS -> 4-byte -> IAC -> SE
		 */
		if (ptr[1] == SB && ptr[2] == TELOPT_NAWS) {
			struct winsize ws;
			if ((ptr + 8) >= end)
				break; /* incomplete, can't process */
			ws.ws_col = (ptr[3] << 8) | ptr[4];
			ws.ws_row = (ptr[5] << 8) | ptr[6];
			ioctl(ttyFd, TIOCSWINSZ, (char *) &ws);
			ptr += 9;
			continue;
		}
		/* skip 3-byte IAC non-SB cmd */
		ptr += 3;
	}

	num_totty = totty - ptr0;
	*pnum_totty = num_totty;
	/* The difference between ptr and totty is number of iacs
	 we removed from the stream. Adjust buf1 accordingly */
	if ((ptr - totty) == 0) /* 99.999% of cases */
		return ptr0;
	ts += ptr - totty;
	tsLen -= ptr - totty;
	/* Move chars meant for the terminal towards the end of the buffer */
	return (unsigned char *) memmove(ptr - num_totty, ptr0, num_totty);
}

ssize_t ClientThread::SafeWrite(int fd, const void *buf, size_t count) {
	ssize_t n;

	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}
size_t ClientThread::IacSafeWrite(int fd, const char *buf, size_t count) {
	const char *IACptr;
	size_t wr, rc, total;

	total = 0;
	while (1) {
		if (count == 0)
			return total;
		if (*buf == (char) IAC) {
			const char IACIAC[] = { IAC, IAC };
			rc = SafeWrite(fd, IACIAC, 2);
			if (rc != 2)
				break;
			buf++;
			total++;
			count--;
			continue;
		}
		/* count != 0, *buf != IAC */
		IACptr = (const char *) memchr(buf, IAC, count);
		wr = count;
		if (IACptr)
			wr = IACptr - buf;
		rc = SafeWrite(fd, buf, wr);
		if (rc != wr)
			break;
		buf += rc;
		total += rc;
		count -= rc;
	}
	/* here: rc - result of last short write */
	if ((ssize_t) rc < 0) { /* error? */
		if (total == 0)
			return rc;
		rc = 0;
	}
	return total + rc;
}

void ClientThread::SubProcess(int &ttyfd, const char *ttyName) {
	int pid;
	struct termios termbuf;
	//子进程开始执行
	setenv("TERM", this->type.c_str(), 1);
	setsid();
	close(0);
	ttyfd = open(ttyName, O_RDWR, 0666);
	dup2(0, 1);
	dup2(0, 2);
	pid = getpid();
	tcsetpgrp(0, pid); /* switch this tty's process group to us */
	tcgetattr(0, &termbuf);
	termbuf.c_lflag |= ECHO;
	termbuf.c_oflag |= ONLCR | XTABS;
	termbuf.c_iflag |= ICRNL;
	termbuf.c_iflag &= ~IXOFF;
	/*termbuf.c_lflag &= ~ICANON;*/
	tcsetattr(STDIN_FILENO, TCSANOW, &termbuf);
	print_login_issue("/etc/issue.net", ttyName);
	//启动登陆程序
	char login[] = "/bin/login";
	char *login_argv[2] = { login, NULL };
	execvp("/bin/login", (char**) (login_argv));
}

void ClientThread::MainProcess(int ptyfd) {
	/**
	 * buf1存放从socket->ptyfd的数据
	 * buf2存放从ptyfd->socket的数据
	 */
	int buf1Len = 0, buf2Len = 0;
	unsigned char buf1[BUFSIZE], buf2[BUFSIZE];
	unsigned char *ptrBuf1 = buf1, *ptrBuf2 = buf2;

	int trycount = 0;
	int fdMax = clientSocket > ptyfd ? clientSocket : ptyfd;
	fd_set rdfdset, wrfdset;
	while (1) {
		FD_ZERO(&wrfdset);
		FD_ZERO(&rdfdset);

		if (buf1Len > 0) {
			FD_SET(ptyfd, &wrfdset);
			fdMax = ptyfd > fdMax ? ptyfd : fdMax;
		}
		if (buf2Len > 0) {
			FD_SET(clientSocket, &wrfdset);
			fdMax = clientSocket > fdMax ? clientSocket : fdMax;
		}
		if (buf1Len < BUFSIZE) {
			FD_SET(clientSocket, &rdfdset);
			fdMax = clientSocket > fdMax ? clientSocket : fdMax;
		}
		if (buf2Len < BUFSIZE) {
			FD_SET(ptyfd, &rdfdset);
			fdMax = ptyfd > fdMax ? ptyfd : fdMax;
		}

		struct timeval timeout;
		timeout.tv_sec = 30;
		timeout.tv_usec = 0;
		int count = select(fdMax + 1, &rdfdset, &wrfdset, NULL, &timeout);

		if (count == 0) {
			LOG("1")
			continue;
		} else if (count < 0) {
			EXIT_MAIN_PROCESS("select error!")
		} else {
			LOG("2")
			//如果socket中可以读取数据，则把数据放入buf1中
			if (FD_ISSET(clientSocket, &rdfdset)) {
				LOG("3")
				count = SafeReadSocket(clientSocket, ptrBuf1, 256);
				if (count <= 0) {
					EXIT_MAIN_PROCESS("read from socket!")
				} else {
					ptrBuf1 = ptrBuf1 + count;
					buf1Len = buf1Len + count;
				}
			}
			//如果ptyfd中的数据可以读取，则把数据放入buf2中
			if (FD_ISSET(ptyfd, &rdfdset)) {
				LOG("4")
				int retry = 0;
				count = SafeReadPtyfd(ptyfd, ptrBuf2, 256, &retry);
				if (count < 0) {
					if (retry == 0 || trycount++ > 10) {
						EXIT_MAIN_PROCESS("read from ptyfd!")
					}
				} else {
					trycount = 0;
					ptrBuf2 = ptrBuf2 + count;
					buf2Len = buf2Len + count;
				}
			}
			//判断ptyfd是否可以写入数据，如果可以则把buf1写入ptyfd
			if (FD_ISSET(ptyfd, &wrfdset)) {
				LOG("5")
				if (buf1Len > 0) {
					int _buf1Len;
					unsigned char *_buf1;
					_buf1 = RemoveIacs((unsigned char *) buf1, buf1Len, ptyfd,
							&_buf1Len);
					count = SafeWrite(ptyfd, _buf1, _buf1Len);
					if (count < 0) {
						if (errno != EAGAIN) {
							EXIT_MAIN_PROCESS("write to ptyfd!")
						}
					} else {
						memmove(buf1, _buf1 + count, _buf1Len - count);
						buf1Len = _buf1Len - count;
						ptrBuf1 = buf1 + buf1Len;
					}
				}
			}
			//判断socket是否可以写入数据，如果可以则把buf2写入socket
			if (FD_ISSET(clientSocket, &wrfdset)) {
				LOG("6")
				if (buf2Len > 0) {
					count = IacSafeWrite(clientSocket, (char *) buf2, buf2Len);
					if (count < 0) {
						if (errno != EAGAIN) {
							EXIT_MAIN_PROCESS("write to socket!")
						}
					} else {
						memmove(buf2, buf2 + count, buf2Len - count);
						buf2Len = buf2Len - count;
						ptrBuf2 = buf2 + buf2Len;
					}
				}
			}
		}
	}
}

void ClientThread::Run() {
	int pid = 0;
	int ptyfd = 0;
	int ttyfd = 0;

	char iacs_to_send[] = { IAC, DO, TELOPT_ECHO, IAC, DO, TELOPT_NAWS,
	/* This requires telnetd.ctrlSQ.patch (incomplete) */
	/*IAC, DO, TELOPT_LFLOW,*/IAC, WILL, TELOPT_ECHO, IAC, WILL, TELOPT_SGA };

	char clientIp[128];
	sprintf(clientIp, "%s", inet_ntoa(clientAddress));

	char screenNum[10];
	ReadScreenNumber(clientSocket, screenNum);

	char ttyName[128];
	if (this->ptyType == "unix98") {
		ptyfd = OpenPtmx(ttyName, clientIp, screenNum);
	} else {
		ptyfd = OpenPty(ttyName, clientIp, screenNum);
	}

	if (ptyfd < 0) {
		char send[128];
		sprintf(send, ERROR_CAN_NOT_FOUND_TTY, clientIp, screenNum, ttyName);
		socket_send(clientSocket, send, strlen(send));
		close(clientSocket);
		return;
	}

	fcntl(ptyfd, F_SETFL, fcntl(ptyfd, F_GETFL) | O_NONBLOCK);
	fcntl(ptyfd, F_SETFD, FD_CLOEXEC);

	socket_options(clientSocket);
	fcntl(clientSocket, F_SETFL, fcntl(clientSocket, F_GETFL) | O_NONBLOCK);
	socket_send(clientSocket, iacs_to_send, sizeof(iacs_to_send));

	pid = fork(); /* NOMMU-friendly */
	if (pid > 0) {
		printf("sid:%d, pid:%d, ttyname:%s, client:%s, screen:%s\r\n", pid,
				getpid(), ttyName, clientIp, screenNum);
		fflush(NULL);
		MainProcess(ptyfd);
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		close(ttyfd);
		close(ptyfd);
		close(clientSocket);
	} else if (pid == 0) { //子进程开始执行
		SubProcess(ttyfd, ttyName);
	} else if (pid < 0) {
		printf("create process error: %d\r\n", pid);
		fflush(NULL);
		close(ttyfd);
		close(ptyfd);
		close(clientSocket);
	}
}
