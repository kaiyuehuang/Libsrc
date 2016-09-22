#ifndef _EPOLL_H_
#define _EPOLL_H_
//#define EPOLL

#ifdef EPOLL
#include <sys/epoll.h>


#define MAXEPOLLSIZE 	10
struct e_msg{
	int  kdpfd;  //epoll 句柄套接字描述符
	int  nfds;    //有数据来之后，唤醒epoll事件，返回活动描述的个数
	int  curfds;    //epoll 监听的集合里面套接字描述符个数
	int listenfd;   //本地服务器监听的描述符
	int connentfd;  //新连接的套接字描述符
	struct epoll_event ev;
	struct epoll_event events[MAXEPOLLSIZE]; 
};

extern struct e_msg *epollmsg;

extern struct e_msg * init_epoll(void);
extern int setnonblocking(int sockfd);
extern int add_fd_epoll(int addfd,struct e_msg *epollmsg);
extern int del_fd_epoll(int addfd,struct e_msg *epollmsg);
extern void handle_epoll(struct e_msg *epollmsg);
#endif
	
#endif
