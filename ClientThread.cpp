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

static const char fmtstr_d[] = "%A, %d %B %Y";
static const char fmtstr_t[] = "%H:%M:%S";

ClientThread::ClientThread() {
	this->local = true;
	this->socket = 0;
	this->needScreen = true;
	this->ptyType = "bsd";
}

ClientThread::~ClientThread() {
}

void ClientThread::SetClientSocket(int socket) {
	this->socket = socket;
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

void ShellExecute(char *shell) {
	FILE *fp = popen(shell, "r");
	pclose(fp);
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

//	char shell[128];
//	memset(shell, 0x00, 128);
//	sprintf(shell, "fuser -k %s", ttyName);
//	ShellExecute(shell);

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

//发送数据
int SocketSend(int Socket, char *sendStr, int sendLen) {
	int rvCount = 0;
	int allC = 0;

	while (1) {
		rvCount = send(Socket, sendStr, sendLen, 0);
		if (rvCount <= 0) {
			return -1;
		}
		allC = allC + rvCount;
		if (allC >= sendLen) {
			break;
		}
	}
	return sendLen;
}

int SocketRecv(int Socket, char *readStr, int readLen) {
	int rvCount = 0;
	int allCout = 0;
	while (1) {
		rvCount = recv(Socket, readStr, readLen, 0);
		if (rvCount <= 0) {
			return -1;
		}
		allCout = allCout + rvCount;
		if (allCout >= readLen) {
			break;
		}
	}
	return readLen;
}

int ReadScreenNumber(int socket, char* screenNum) {
	char num[12];
	int len = recv(socket, num, 12, 0) - 2;
	memcpy(screenNum, num + 2, len);
	screenNum[len] = 0;
	return len;
}

void print_login_issue(const char *issue_file, const char *tty) {
	FILE *fp;
	int c;
	char buf[256 + 1];
	const char *outbuf;
	time_t t;
	struct utsname uts;

	time(&t);
	uname(&uts);
	//向前端显示信息
	puts("\r"); /* start a new line */
	printf(LOG_SERVER_TITLE, tty);

	fp = fopen(issue_file, "r");
	if (!fp)
		return;
	while ((c = fgetc(fp)) != EOF) {
		outbuf = buf;
		buf[0] = c;
		buf[1] = '\0';
		if (c == '\n') {
			buf[1] = '\r';
			buf[2] = '\0';
		}
		if (c == '\\' || c == '%') {
			c = fgetc(fp);
			switch (c) {
			case 's':
				outbuf = uts.sysname;
				break;
			case 'n':
			case 'h':
				outbuf = uts.nodename;
				break;
			case 'r':
				outbuf = uts.release;
				break;
			case 'v':
				outbuf = uts.version;
				break;
			case 'm':
				outbuf = uts.machine;
				break;
				/* The field domainname of struct utsname is Linux specific. */
#if defined(__linux__)
			case 'D':
			case 'o':
				outbuf = uts.domainname;
				break;
#endif
			case 'd':
				strftime(buf, sizeof(buf), fmtstr_d, localtime(&t));
				break;
			case 't':
				strftime(buf, sizeof(buf), fmtstr_t, localtime(&t));
				break;
			case 'l':
				outbuf = tty;
				break;
			default:
				buf[0] = c;
				break;
			}
		}
		//可以加入当前TTY号的显示
		fputs(outbuf, stdout);
	}
	fclose(fp);
	fflush(NULL);
}

ssize_t safe_read_ptyfd(int fd, void *buf, size_t count) {
	ssize_t n;
	do {
		n = read(fd, buf, count);
		if (n < 0) {
			if (errno == EIO) {
				continue;
			}
			if (errno == EINTR) {
				printf("errno: %d\r\n", errno);
				continue;
			}
			if (errno == EAGAIN) {
				continue;
			}
		}
		break;
	} while (true);

	if (n < 0) {
		printf("errno: %d\r\n", errno);
	}

	return n;
}
ssize_t safe_read_socket(int fd, void *buf, size_t count) {
	ssize_t n;
	do {
		n = read(fd, buf, count);
		if (n < 0) {
			if (errno == EINTR) {
				printf("errno: %d\r\n", errno);
				continue;
			}
			if (errno == EAGAIN) {
				continue;
			}
		}
		break;
	} while (true);

	if (n < 0) {
		printf("errno: %d\r\n", errno);
	}

	return n;
}

unsigned char *remove_iacs(unsigned char *ts, int tsLen, int ttyFd,
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
ssize_t safe_write(int fd, const void *buf, size_t count) {
	ssize_t n;

	do {
		n = write(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}
size_t iac_safe_write(int fd, const char *buf, size_t count) {
	const char *IACptr;
	size_t wr, rc, total;

	total = 0;
	while (1) {
		if (count == 0)
			return total;
		if (*buf == (char) IAC) {
			const char IACIAC[] = { IAC, IAC };
			rc = safe_write(fd, IACIAC, 2);
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
		rc = safe_write(fd, buf, wr);
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

void ClientThread::Run() {
	char str[256];
	int pid = 0;
	fd_set rdfdset, wrfdset;
	struct termios termbuf;
	int count = 0;
	int fdMax = 0;
	char recvData[512];
	char shell[128];

	int keepAlive = 1; //设定KeepAlive=1
	int keepIdle = 100; //开始首次KeepAlive探测前的TCP空闭时间
	int keepInterval = 500; //两次KeepAlive探测间的时间间隔
	int keepCount = 3; //判定断开前的KeepAlive探测次数

	struct timeval timeout;

	int ptyfd = 0;
	int ttyfd = 0;

	pid_t sonPid;

	char ttyName[128];

	unsigned char *ptrBuf1;
	unsigned char buf1[BUFSIZE]; //接收socket发过来的数据
	int buf1Len = 0;
	ptrBuf1 = buf1;

	unsigned char *ptrBuf2;
	unsigned char buf2[BUFSIZE]; //发送给socket的数据
	int buf2Len = 0;
	ptrBuf2 = buf2;

	unsigned char LsStr[BUFSIZE];

	char iacs_to_send[] = { IAC, DO, TELOPT_ECHO, IAC, DO, TELOPT_NAWS,
	/* This requires telnetd.ctrlSQ.patch (incomplete) */
	/*IAC, DO, TELOPT_LFLOW,*/IAC, WILL, TELOPT_ECHO, IAC, WILL, TELOPT_SGA };

	char clientIp[128];
	memset(clientIp, 0x00, 128);
	sprintf(clientIp, "%s", inet_ntoa(clientAddress));

	char screenNum[10] = "\0";
	if (needScreen) {
		char chr1[] = { 0xff, 0xf1, 0x18, 0x00, 0x00, 0x00 };
		char chr2[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		SocketSend(socket, chr1, 6);
		ReadScreenNumber(socket, screenNum);
		SocketSend(socket, chr2, 6);
	}

	if (this->ptyType == "unix98") {
		ptyfd = OpenPtmx(ttyName, clientIp, screenNum);
	} else {
		ptyfd = OpenPty(ttyName, clientIp, screenNum);
	}

	if (ptyfd < 0) {
		char send[128];
		sprintf(send, ERROR_CAN_NOT_FOUND_TTY, clientIp, screenNum, ttyName);
		SocketSend(socket, send, strlen(send));
		close(socket);
		return;
	}

	printf("ttyname:'%s', client:'%s', screen:'%s'\r\n", ttyName, clientIp,
			screenNum);

	fcntl(ptyfd, F_SETFL, fcntl(ptyfd, F_GETFL) | O_NONBLOCK);
	fcntl(ptyfd, F_SETFD, FD_CLOEXEC);

	//<2>设置socket状态,保持长连接  // 设置KeepAlive参数
	if (setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (void*) &keepAlive,
			sizeof(keepAlive)) == -1) {
	}
	if (setsockopt(socket, SOL_TCP, TCP_KEEPIDLE, (void *) &keepIdle,
			sizeof(keepIdle)) == -1) {
	}
	if (setsockopt(socket, SOL_TCP, TCP_KEEPINTVL, (void *) &keepInterval,
			sizeof(keepInterval)) == -1) {
	}
	if (setsockopt(socket, SOL_TCP, TCP_KEEPCNT, (void *) &keepCount,
			sizeof(keepCount)) == -1) {
	}
	fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);

	SocketSend(socket, iacs_to_send, sizeof(iacs_to_send));

	fflush(NULL);
	::signal(SIGPIPE, SIG_IGN); //忽略socket错误产生的SIGPIPE信号,防止进程异常退出
	::signal(SIGCHLD, SIG_IGN); //忽略子进程退出信号
	::signal(SIGSEGV, SIG_IGN); //另一端断开
	//::signal(SIGSEGV, &sig_int); //另一端断开
	pid = fork(); /* NOMMU-friendly */
	if (pid > 0) {
		FD_ZERO(&rdfdset);
		FD_ZERO(&wrfdset);

		buf1Len = 0;
		buf2Len = 0;
		sonPid = pid;
		while (1) {
			FD_ZERO(&rdfdset);
			FD_ZERO(&wrfdset);

			if (buf1Len > 0) {
				FD_SET(ptyfd, &wrfdset);
				if (ptyfd > fdMax) {
					fdMax = ptyfd;
				}
			}
			if (buf2Len > 0) {
				FD_SET(socket, &wrfdset);
				if (socket > fdMax) {
					fdMax = socket;
				}
			}
			if (buf1Len < BUFSIZE) {
				FD_SET(socket, &rdfdset);
				if (socket > fdMax) {
					fdMax = socket;
				}
			}
			if (buf2Len < BUFSIZE) {
				FD_SET(ptyfd, &rdfdset);
				if (ptyfd > fdMax) {
					fdMax = ptyfd;
				}
			}
			//socket是否有读写，超时时间30秒
			count = 0;
			if (fdMax < 0) {
				fdMax = 0;
			}
			timeout.tv_sec = 30; //超时判断为30秒
			timeout.tv_usec = 0;

			count = select(fdMax + 1, &rdfdset, &wrfdset, NULL, &timeout);
			if (count == 0) {
				continue;
			} else if (count < 0) {
				kill(sonPid, SIGKILL);
				waitpid(sonPid, NULL, 0);
				close(ttyfd);
				close(ptyfd);
				close(socket);
				memset(shell, 0x00, 128);
				sprintf(shell, "fuser -k %s", ttyName);
				ShellExecute(shell);
			} else {
				count = 0;
				memset(str, 0x00, 256);
				if (FD_ISSET(socket, &rdfdset)) {
					memset(recvData, 0x00, 512);
					count = safe_read_socket(socket, recvData, 256); //向buf1中读入socket发来数据
					memcpy(ptrBuf1, recvData, count);
					if (count < 0) {
						printf("read from socket error!\r\n");
						break; //关闭当前连接
					} else {
						ptrBuf1 = ptrBuf1 + count;
						buf1Len = buf1Len + count;
					}
				}
				count = 0;
				if (FD_ISSET(ptyfd, &rdfdset)) {
					count = safe_read_ptyfd(ptyfd, ptrBuf2, 256);
					if (count < 0) {
						printf("read from ptyfd error!\r\n", count);
						break; //关闭当前连接
					} else {
						ptrBuf2 = ptrBuf2 + count;
						buf2Len = buf2Len + count;
					}
				}
				count = 0;
				if ((FD_ISSET(ptyfd, &wrfdset)) && (buf1Len > 0)) {
					int num_totty;
					unsigned char *ptr;
					ptr = remove_iacs((unsigned char *) buf1, buf1Len, ptyfd,
							&num_totty); //去掉特殊字符
					count = safe_write(ptyfd, ptr, num_totty);
					if (count < 0) {
						if (errno != EAGAIN) //应用程序现在没有数据可写请稍后再试
						{
							printf("write to ptyfd error!\r\n");
							break; //关闭当前连接
						}
					} else {
						memcpy(LsStr, ptr + count, num_totty - count);
						buf1Len = num_totty - count;
						memcpy(buf1, LsStr, buf1Len);
						ptrBuf1 = buf1 + buf1Len;
					}
				}
				//判断socket是否可以写入数据，如果可以则把buf2写入socket
				count = 0;
				if ((FD_ISSET(socket, &wrfdset)) && (buf2Len > 0)) {
					count = iac_safe_write(socket, (char *) buf2, buf2Len);
					if (count < 0) {
						if (errno != EAGAIN) //如果不能写入，则继续下一步检查
						{
							printf("write to socket error!\r\n");
							break; //关闭当前连接
						}
					} else {
						memcpy(LsStr, buf2 + count, buf2Len - count);
						buf2Len = buf2Len - count;
						memcpy(buf2, LsStr, buf2Len);
						ptrBuf2 = buf2 + buf2Len;
					}
				}
			}
		}
		kill(sonPid, SIGKILL);
		waitpid(sonPid, NULL, 0);
		close(ttyfd);
		close(ptyfd);
		close(socket);
	} else if (pid == 0) { //子进程开始执行
		setenv("TERM", this->type.c_str(), 1);
		setsid();
		close(0);
		ttyfd = open(ttyName, O_RDWR, 0666);
		dup2(0, 1);
		dup2(0, 2);
		pid = getpid();
		tcsetpgrp(0, pid); /* switch this tty's process group to us */
		tcgetattr(0, &termbuf);
		termbuf.c_lflag |= ECHO; /* if we use readline we dont want this */
		termbuf.c_oflag |= ONLCR | XTABS;
		termbuf.c_iflag |= ICRNL;
		termbuf.c_iflag &= ~IXOFF;
		/*termbuf.c_lflag &= ~ICANON;*/
		tcsetattr(STDIN_FILENO, TCSANOW, &termbuf);

		print_login_issue("/etc/issue.net", ttyName);

		//启动登陆程序
		char login[] = "/bin/login";
		char *login_argv[2] = { login, NULL };
		execvp("/bin/login", (char **) login_argv);
	} else if (pid < 0) {
		printf("create process error: %d\r\n", pid);
		close(ttyfd);
		close(ptyfd);
		close(socket);
	}
}

void ClientThread::SetPtyType(const string& pty) {
	this->ptyType = pty;
}

void ClientThread::SetNeedScreen(bool need) {
	this->needScreen = need;
}
