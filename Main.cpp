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
	printf("绑定服务配置文件：%s\r\n", TELNET_CFG);
	int port = config.GetInteger("port", 3231);
	int count = config.GetInteger("maxcount", 256);
	string type = config.GetString("vttype", "vt100");
	printf("绑定服务端口：%d\r\n仿真类型：%s\r\n", port, type.c_str());

	Properties ttys;
	string ttymap = "";
	if (config.GetString("maptype", "") == "star"){
		ttymap = config.GetString("ttymap", "/etc/stelnetd.conf");
		printf("使用实达的绑定映射文件：%s\r\n", ttymap.c_str());
		ttys.LoadTable(ttymap);
	} else {
		ttymap = config.GetString("ttymap", TELNET_CFG);
		printf("绑定映射文件：%s\r\n", ttymap.c_str());
		ttys.Load(ttymap);
	}
	printf("PTY设备类型：%s\r\n", config.GetString("device").c_str());

	TerminalServer server;
	server.SetPort(port);
	server.SetCount(count);
	server.SetType(type);
	server.SetTtyConfig(&ttys);
	server.SetPtyType(config.GetString("device"));
	server.SetNeedScreen(config.GetBoolean("screenNum"));

	if (config.GetBoolean("screenNum")) {
		printf("仿真终端需要发送屏号\r\n");
	} else {
		printf("仿真终端不需要发送屏号\r\n");
	}

	printf("绑定服务正在运行！\r\n");

	ThreadCreator::StartThread(&server)->Wait();

	printf("绑定服务运行结束！\r\n");

	return 0;
}
