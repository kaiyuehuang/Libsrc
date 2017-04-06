#include  <unistd.h>
#include  <sys/types.h>      /* basic system data types */
#include  <sys/socket.h>     /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <fcntl.h>     /* nonblocking */
#include <sys/resource.h> /*setrlimit */
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "epoll.h"
/******设置套接字为非阻塞工作方式***********/
int setnonblocking(int sockfd){    
	return fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK);    
}

/********添加描述符到epoll里面***********/
int Epoll_AddSockfd(Epoll_t *ep,int sockfd){
	if (sockfd < 0){                    
		perror("accept error");          
		return -1;  
	}      
	if (ep->curfds >= ep->max){ 
		fprintf(stderr, "too many connection, more than %d\n", ep->max); 
		close(sockfd);     
		return -1;   
	}   
	if (setnonblocking(sockfd) < 0) {        
		perror("setnonblock error");    
	} 
	
	ep->ev.events = EPOLLIN | EPOLLET;   	
	ep->ev.data.fd = sockfd;    
	
	if (epoll_ctl(ep->kdpfd, EPOLL_CTL_ADD, sockfd, &ep->ev) < 0){        
		fprintf(stderr, "epoll set insertion error: fd=%d\n", sockfd); 
		return -1;    
	}    
	ep->curfds++;    
	printf("add new fd to epoll %d\n",sockfd); 
	return 1;
}

int Epoll_DeleteSockfd(Epoll_t *ep,int sockfd){
	if (sockfd < 0)                {                    
		perror("accept error");          
		return -1;  
	}      
	if(ep->curfds <= 0) { 
		fprintf(stderr, "not fd  connection\n"); 
		close(sockfd);     
		return -1;   
	}   
	ep->ev.events = EPOLLIN | EPOLLET;    
	ep->ev.data.fd = -1;    
	if (epoll_ctl(ep->kdpfd, EPOLL_CTL_DEL, sockfd, &ep->ev) < 0){        
		fprintf(stderr, "epoll set insertion error: fd=%d\n", sockfd); 
		return -1;    
	}    
	ep->curfds--;    
	printf("%d\n",sockfd); 
	return 0;
}
void Epoll_pthreadListen(void *arg){
	int n;
	int len = sizeof(struct sockaddr_in);
	struct sockaddr_in cliaddr;
	Epoll_t *ep = (Epoll_t *)arg;
	while(1){        /* 等待有事件发生 */        
		ep->nfds = epoll_wait(ep->kdpfd, ep->events, ep->curfds, -1);        
		if (ep->nfds == -1){            
			perror("epoll_wait--");
			usleep(1000);
			continue;       
		}        /* 处理所有事件 */      
		
		for (n = 0; n < ep->nfds; ++n){      
			if (ep->events[n].data.fd == ep->listenfd){               
				ep->connentfd = accept(ep->listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);                
				printf("connet ip : %s\n",inet_ntoa(cliaddr.sin_addr));
				ep->epollConnect_sockfd(ep,ep->events[n].data.fd);
				continue;      
			}
			else if(ep->events[n].events&EPOLLIN)  //处理客户端发送过来的数据
			{
		
				ep->recvEpoll_sock(ep,ep->events[n].data.fd);
			}
		}
	
	} 

}
Epoll_t * Epoll_Init(int max,int epollConnect_sockfd(Epoll_t *ep,int aliveSOckfd),void recvEpoll_sock(Epoll_t *ep,int aliveSOckfd)){
	Epoll_t *ep = (Epoll_t *)calloc(1,sizeof(Epoll_t));
	if(ep==NULL){
		perror("malloc  ep failed");
		return NULL;
	}
	ep->events = (struct epoll_event *)calloc(1,sizeof(struct epoll_event)*max);
	if(ep->events==NULL){
		goto exit0;
	}
	ep->max = max;
	ep->kdpfd = epoll_create(max);
	ep->epollConnect_sockfd = epollConnect_sockfd; 
	ep->recvEpoll_sock = recvEpoll_sock;
	return ep;
exit0:
	free(ep);
	ep=NULL;
	return NULL;
}
int main(void){

	return 0;
}
