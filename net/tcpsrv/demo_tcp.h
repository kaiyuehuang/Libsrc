#ifndef _DEMO_TCP_H
#define _DEMO_TCP_H

#include <netdb.h>

/*******************************************************
函数功能: 创建一个tcp连接客户端
参数: serverip 服务器的ip地址 port 服务器监听的端口
返回值: -1连接失败 大于0的值连接成功之后
********************************************************/
extern int create_client(char *serverip,int port);

/*******************************************************
函数功能: 创建一个tcp监听服务器
参数: bindip绑定的IP 地址 port监听的端口号
返回值: -1监听失败 大于0的值监听成功
********************************************************/
extern int create_server(char *bindip,int port);

extern struct hostent *timeGesthostbyname(char *hostname,int timeout);
extern int checkNetwork(int timeout);
//获取网卡的IP地址
extern int GetNetworkcardIp(char *NetName,char *IP);
//获取当前网络的外网IP地址
extern int getwlanip(char *wlanip);

#endif
