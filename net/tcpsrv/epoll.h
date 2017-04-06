#ifndef _EPOLL_H_
#define _EPOLL_H_

#include <sys/epoll.h>
typedef struct epoll{
	int  kdpfd;  //epoll ����׽���������
	int  nfds;    //��������֮�󣬻���epoll�¼������ػ�����ĸ���
	int  curfds;    //epoll �����ļ��������׽�������������
	int listenfd;   //���ط�����������������
	int connentfd;  //�����ӵ��׽���������
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
