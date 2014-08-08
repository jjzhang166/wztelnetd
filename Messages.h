#ifndef MESSAGES_H
#define MESSAGES_H
#define ERROR_CAN_NOT_OPEN_PORT "端口无法打开：%d，请确认该端口是否已经被占用！\r\n"
#define ERROR_CAN_NOT_FOUND_TTY "未找到空闲可用的tty设备(%s.%s:%s)!"
#define ERROR_CAN_NOT_FOUND_CFG "%s.%s在绑定配置文件中没有设置！\r\n"

#define LOG_SERVER_CFG_FILE "绑定服务配置文件：%s\r\n"
#define LOG_SERVER_PORT "绑定服务端口：%d\r\n"
#define LOG_SERVER_TTY_TYPE "仿真类型：%s\r\n"

#define LOG_SERVER_NEED_SCREEN_NUM "仿真终端需要发送屏号\r\n"
#define LOG_SERVER_NEED_NOT_SCREEN_NUM "仿真终端不需要发送屏号\r\n"
#define LOG_SERVER_RUNNING "绑定服务正在运行！\r\n"
#define LOG_SERVER_OVER "绑定服务运行结束！\r\n"

#define LOG_SERVER_SD_MAP_FILE "使用实达的绑定映射文件：%s\r\n"
#define LOG_SERVER_MAP_FILE "绑定映射文件：%s\r\n"
#define LOG_SERVER_PTY_TYPE "PTY设备类型：%s\r\n"

#define LOG_SERVER_TITLE "欢迎使用汇金终端仿真软件V4.0! 本终端tty名称为:%s\n"
#endif
