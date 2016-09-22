#ifndef _EPOLL_H_
#define _EPOLL_H_
//#define EPOLL

#ifdef EPOLL
#include <sys/epoll.h>


#define MAXEPOLLSIZE 	10
struct e_msg{
	int  kdpfd;  //epoll ����׽���������
	int  nfds;    //��������֮�󣬻���epoll�¼������ػ�����ĸ���
	int  curfds;    //epoll �����ļ��������׽�������������
	int listenfd;   //���ط�����������������
	int connentfd;  //�����ӵ��׽���������
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
