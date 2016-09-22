#include <signal.h>
#include <stdio.h>
#include <string.h>
#include  <sys/types.h>      /* basic system data types */
#include  <sys/socket.h>     /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <sys/resource.h> /*setrlimit */
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include<signal.h>   
#include <setjmp.h>
#include <time.h>
#include <netdb.h>

#include "sock_fd.h"
#include "config.h"

#ifdef DNS_TIMEOUT
static sigjmp_buf jmpbuf;
static void alarm_func(int sig)
{
	printf("time out\n");
	siglongjmp(jmpbuf,1);
}

static struct hostent *timeGesthostbyname(char *ip,char *hostname,int timeout)
{
	struct hostent *h=NULL;
	signal(SIGALRM,alarm_func);
	
	if(sigsetjmp(jmpbuf,1)!=0)
	{
		alarm(0); //time out
		printf("sigsetjmp remove \n");
		signal(SIGALRM,SIG_IGN);
		return NULL;
	}
	alarm(timeout);
	h=gethostbyname(hostname);
	signal(SIGALRM,SIG_IGN);
	return h;
}
#endif

/*************************************
根据域名获取远程服务器IP
***************************************/
int get_serverip(char *ip,char *hostname)
{
#ifdef STATIC_NET
	strcpy(ip,STATIC_NET_IP);	

#elif defined DNS_NET
	struct hostent *h;
	if((h=gethostbyname(hostname))==NULL)
	{
		perror("gethostbyname failed ");
		goto err;
	}
	strcpy(ip,(const char *)inet_ntoa(*((struct in_addr *)h->h_addr)));
	if(!strcmp(ip,DNS_IP_ERROR))
	{
		printf("dns ip error ,get server failed\n");
		strcpy(ip,DNS_IP_ERROR);	
		return -1;
	}

#elif defined DNS_TIMEOUT
	struct hostent *h = timeGesthostbyname(ip,hostname,5);
	if(h==NULL)
	{
		perror("gethostbyname failed ");
		goto err;
	}
	if(!strcmp(ip,DNS_IP_ERROR))
	{
		printf("dns ip error ,get server failed\n");
		goto err;
	}
	strcpy(ip,(const char *)inet_ntoa(*((struct in_addr *)h->h_addr)));
#else
	printf("not config get serverip type\n");
	strcpy(ip,"0.0.0.0");
#endif
	return 0;
err:
	strcpy(ip,"0.0.0.0");
	return -1;
}

#ifdef CONFIG_CLIENT 
/*************************************************
创建本地客户端
**************************************************/
int create_client(char *serverip,int port)
{
	int fd;
	struct sockaddr_in server_addr; 
	struct timeval tv = {5, 0};
	fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd == -1)
	{
		perror("socket fail!");
		return fd;
	}	
	/********描述服务器套接字信息********/
	memset(&server_addr,0,sizeof(struct sockaddr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr =  inet_addr(serverip);
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
	if(connect(fd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) ==-1)	
	{		
		close(fd);
		return -1;	
	}
	return fd;
}
#endif

#ifdef CONFIG_SERVER
/*********创建本地服务器，监听客户端连接**********/
int create_server(char *bindip,int port)
{
	int listenfd;
	struct sockaddr_in servaddr; 
	bzero(&servaddr, sizeof(struct sockaddr));    
	servaddr.sin_family = AF_INET;     

	if(bindip==NULL)
		servaddr.sin_addr.s_addr = htonl (INADDR_ANY);    
	else
		servaddr.sin_addr.s_addr = inet_addr(bindip);
	
	servaddr.sin_port = htons (port);    
	listenfd = socket(AF_INET, SOCK_STREAM, 0); 
	
	if (listenfd == -1) 
	{        
		perror("can't create socket file");       
		return listenfd;    
	}    
	int opt = 1;    
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));    
 
	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) == -1)     
	{        
		perror("bind error");        
		return listenfd;    
	}     
	if (listen(listenfd, LISTENMAX) == -1)     
	{        
		perror("listen error");        
		return listenfd;    
	}    /* 创建 epoll 句柄，把监听 socket 加入到 epoll 集合里 */    
	return listenfd;
}
#endif

