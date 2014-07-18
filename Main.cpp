/*
 * Main.cpp
 *
 *  Created on: Jan 21, 2013
 *      Author: zhangbo
 */

#include "TerminalServer.h"
#include "Properties.h"
#include <stdio.h>
#include "ThreadCreator.h"

#define ETC_PATH "/etc/"
#define TELNET_CFG ETC_PATH "wztelnetd.cfg"

int main() {
	Properties config;
	config.Load(TELNET_CFG);
	printf("�󶨷��������ļ���%s\r\n", TELNET_CFG);
	int port = config.GetInteger("port", 3231);
	int count = config.GetInteger("maxcount", 256);
	string type = config.GetString("vttype", "vt100");
	printf("�󶨷���˿ڣ�%d\r\n�������ͣ�%s\r\n", port, type.c_str());

	Properties ttys;
	string ttymap = "";
	if (config.GetString("maptype", "") == "star"){
		ttymap = config.GetString("ttymap", "/etc/stelnetd.conf");
		printf("ʹ��ʵ��İ�ӳ���ļ���%s\r\n", ttymap.c_str());
		ttys.LoadTable(ttymap);
	} else {
		ttymap = config.GetString("ttymap", TELNET_CFG);
		printf("��ӳ���ļ���%s\r\n", ttymap.c_str());
		ttys.Load(ttymap);
	}
	printf("PTY�豸���ͣ�%s\r\n", config.GetString("device").c_str());

	TerminalServer server;
	server.SetPort(port);
	server.SetCount(count);
	server.SetType(type);
	server.SetTtyConfig(&ttys);
	server.SetPtyType(config.GetString("device"));
	server.SetNeedScreen(config.GetBoolean("screenNum"));

	if (config.GetBoolean("screenNum")) {
		printf("�����ն���Ҫ��������\r\n");
	} else {
		printf("�����ն˲���Ҫ��������\r\n");
	}

	printf("�󶨷����������У�\r\n");

	ThreadCreator::StartThread(&server)->Wait();

	printf("�󶨷������н�����\r\n");

	return 0;
}
