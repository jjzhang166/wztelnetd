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
#include "Messages.h"

#define ETC_PATH "/etc/"
#define TELNET_CFG ETC_PATH "wztelnetd.cfg"

int main() {
	Properties config;
	config.Load(TELNET_CFG);
	printf(LOG_SERVER_CFG_FILE, TELNET_CFG);
	int port = config.GetInteger("port", 3231);
	int count = config.GetInteger("maxcount", 256);
	string type = config.GetString("vttype", "vt100");
	printf(LOG_SERVER_PORT, port);
	printf(LOG_SERVER_TTY_TYPE, type.c_str());

	Properties ttys;
	string ttymap = "";
	if (config.GetString("maptype", "") == "star"){
		ttymap = config.GetString("ttymap", "/etc/stelnetd.conf");
		printf(LOG_SERVER_SD_MAP_FILE, ttymap.c_str());
		ttys.LoadTable(ttymap);
	} else {
		ttymap = config.GetString("ttymap", TELNET_CFG);
		printf(LOG_SERVER_MAP_FILE, ttymap.c_str());
		ttys.Load(ttymap);
	}
	printf(LOG_SERVER_PTY_TYPE, config.GetString("device").c_str());

	TerminalServer server;
	server.SetPort(port);
	server.SetCount(count);
	server.SetType(type);
	server.SetTtyConfig(&ttys);
	server.SetPtyType(config.GetString("device"));
	server.SetNeedScreen(config.GetBoolean("screenNum"));

	if (config.GetBoolean("screenNum")) {
		printf(LOG_SERVER_NEED_SCREEN_NUM);
	} else {
		printf(LOG_SERVER_NEED_NOT_SCREEN_NUM);
	}

	printf(LOG_SERVER_RUNNING);

	ThreadCreator::StartThread(&server)->Wait();

	printf(LOG_SERVER_OVER);

	return 0;
}
