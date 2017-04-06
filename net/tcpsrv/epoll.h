#ifndef _EPOLL_H_
#define _EPOLL_H_

#include <sys/epoll.h>
typedef struct epoll{
	int  kdpfd;  //epoll 句柄套接字描述符
	int  nfds;    //有数据来之后，唤醒epoll事件，返回活动描述的个数
	int  curfds;    //epoll 监听的集合里面套接字描述符个数
	int listenfd;   //本地服务器监听的描述符
	int connentfd;  //新连接的套接字描述符
	int max;
	struct epoll_event ev;
	struct epoll_event *events; 
	int (*epollConnect_sockfd)(struct epoll *ep,int aliveSOckfd);
	void (*recvEpoll_sock)(struct epoll *ep,int aliveSOckfd);
}Epoll_t;


extern Epoll_t * Epoll_Init(int max,int epollConnect_sockfd(Epoll_t *ep,int aliveSOckfd),void recvEpoll_sock(Epoll_t *ep,int aliveSOckfd));
extern int setnonblocking(int sockfd);
extern int Epoll_AddSockfd(Epoll_t *ep,int sockfd);
extern int Epoll_DeleteSockfd(Epoll_t *ep,int sockfd);
extern void Epoll_pthreadListen(void *arg);
	
#endif
