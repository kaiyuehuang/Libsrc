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

#ifdef EPOLL
#include "pool/que_mempool.h"
#include "pool/pthread_pool.h"
#include "common/msgstruct.h"
#include "send_recv.h"
#include "sock_fd.h"
#include "epoll.h"
#include "netconfig.h"
#include "command.h"

struct e_msg *epollmsg;

/******设置套接字为非阻塞工作方式***********/
int setnonblocking(int sockfd)
{    
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) 
	{        
		return -1;    
	}    
	return 0;
}

/********添加描述符到epoll里面***********/
int add_fd_epoll(int addfd,struct e_msg *epollmsg)
{
	if (addfd < 0)                
	{                    
		perror("accept error");          
		return -1;  
	}      
	if (epollmsg->curfds >= MAXEPOLLSIZE) 
	{ 
		fprintf(stderr, "too many connection, more than %d\n", MAXEPOLLSIZE); 
		close(addfd);     
		return -1;   
	}   
	if (setnonblocking(addfd) < 0) 
	{        
		perror("setnonblock error");    
	} 
	
	epollmsg->ev.events = EPOLLIN | EPOLLET;   	
	epollmsg->ev.data.fd = addfd;    
	
	if (epoll_ctl(epollmsg->kdpfd, EPOLL_CTL_ADD, addfd, &epollmsg->ev) < 0)     
	{        
		fprintf(stderr, "epoll set insertion error: fd=%d\n", addfd); 
		return -1;    
	}    
	epollmsg->curfds++;    
	printf("add new fd to epoll %d\n",addfd); 
	return 1;
}



int del_fd_epoll(int delfd, struct e_msg *epollmsg)
{
	if (delfd < 0)                
	{                    
		perror("accept error");          
		return -1;  
	}      
	if(epollmsg->curfds <= 0) 
	{ 
		fprintf(stderr, "not fd  connection\n"); 
		close(delfd);     
		return -1;   
	}   
	epollmsg->ev.events = EPOLLIN | EPOLLET;    
	epollmsg->ev.data.fd = -1;    
	if (epoll_ctl(epollmsg->kdpfd, EPOLL_CTL_DEL, delfd, &epollmsg->ev) < 0)     
	{        
		fprintf(stderr, "epoll set insertion error: fd=%d\n", delfd); 
		return -1;    
	}    
	epollmsg->curfds--;    
	printf(" del fd from epoll %d\n",delfd); 
	return 1;
}
void handle_epoll(struct e_msg *epollmsg)
{
	int n;
	int len = sizeof(struct sockaddr_in);
	struct sockaddr_in cliaddr;
	epollmsg->kdpfd = epoll_create(MAXEPOLLSIZE); 
	epollmsg->listenfd =create_server_fd(NULL,LOCAL_PORT);      // for accept  app login
	add_fd_epoll(epollmsg->listenfd,epollmsg);

	while(1)
	{        /* 等待有事件发生 */        
		epollmsg->nfds = epoll_wait(epollmsg->kdpfd, epollmsg->events, epollmsg->curfds, -1);        
		if (epollmsg->nfds == -1)        
		{            
			perror("epoll_wait--");
			sleep(1);
			continue;       
		}        /* 处理所有事件 */      
		
		for (n = 0; n < epollmsg->nfds; ++n)        
		{      
			if (epollmsg->events[n].data.fd == epollmsg->listenfd)             
			{               
				epollmsg->connentfd = accept(epollmsg->listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);                
				printf("connet ip : %s\n",inet_ntoa(cliaddr.sin_addr));
  				add_fd_epoll(epollmsg->connentfd,epollmsg);
				continue;      
			}
			else if(epollmsg->events[n].events&EPOLLIN)  //处理客户端发送过来的数据
			{
		
				pool_add_task(handle_msg,&epollmsg->events[n].data.fd);
			}
		}
	
	} 

}
struct e_msg * init_epoll(void)
{
	struct e_msg *epollmsg = (struct e_msg *)malloc(sizeof(struct e_msg));
	if(epollmsg==NULL)
	{
		perror("malloc  epollmsg failed");
		return NULL;
	}
	memset(epollmsg,0,sizeof(struct e_msg));
	return epollmsg;
}
#endif
