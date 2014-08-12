/*
 * Util.cpp
 *
 *  Created on: Aug 10, 2014
 *      Author: zhangbo
 */
#include "Messages.h"
#include <stdio.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <time.h>

static const char fmtstr_d[] = "%A, %d %B %Y";
static const char fmtstr_t[] = "%H:%M:%S";

void socket_options(int sock) {
	int keepAlive = 1; //非零值，启用KeepAlive机制
	int keepIdle = 300; //开始首次KeepAlive探测前的TCP空闭时间（秒）
	int keepInterval = 500; //两次KeepAlive探测间的时间间隔
	int keepCount = 3; //判定断开前的KeepAlive探测次数

	if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void*) &keepAlive,
			sizeof(keepAlive)) == -1) {
	}
	if (setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (void *) &keepIdle,
			sizeof(keepIdle)) == -1) {
	}
	if (setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (void *) &keepInterval,
			sizeof(keepInterval)) == -1) {
	}
	if (setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (void *) &keepCount,
			sizeof(keepCount)) == -1) {
	}
}

//发送数据
int socket_send(int Socket, char *sendStr, int sendLen) {
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

int socket_recv(int Socket, char *readStr, int readLen) {
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

void shell_execute(char *shell) {
	FILE *fp = popen(shell, "r");
	pclose(fp);
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
