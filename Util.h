/*
 * Util.h
 *
 *  Created on: Aug 10, 2014
 *      Author: zhangbo
 */

#ifndef UTIL_H_
#define UTIL_H_

void socket_options(int sock);

int socket_send(int Socket, char *sendStr, int sendLen);

int socket_recv(int Socket, char *readStr, int readLen);

void print_login_issue(const char *issue_file, const char *tty);

void shell_execute(char *shell);

#endif /* UTIL_H_ */
