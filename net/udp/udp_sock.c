#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>

void init_addr(struct sockaddr_in *addr, char *ip,  int port)
{
	 addr->sin_family = AF_INET;
	  
	 addr->sin_port = htons(port);
	  if(ip == NULL)
	 	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	else
		addr->sin_addr.s_addr = inet_addr(ip);
}

/*******************************************************
函数功能:根据网卡名字获取网卡的IP地址
参数:net_name网卡名字 IP获取出来的ip地址
返回值: 0 获取成功 -1 获取失败
********************************************************/
int get_networkcard_ip(char *net_name,char *IP)
{
	struct sockaddr_in server_addr;
	struct ifreq ifr;
	strncpy(ifr.ifr_name, net_name, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	if(fd == -1)	
	{		
		perror("create udp socket fail!");
		return -1;	
	}
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		perror("ioctl error\n");
        return -1;
    }
	memcpy(&server_addr, &ifr.ifr_addr, sizeof(server_addr));
   	sprintf(IP,"%s", (char *)inet_ntoa(server_addr.sin_addr));
	if(fd>0)
		close(fd);
   	return 0;
}

static void set_sockfd_timeout(int sockfd,int time)
{
	struct timeval tv_out;
	tv_out.tv_sec = time;
	tv_out.tv_usec = 0;
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
}

/*******************************************************
函数功能:创建一个监听的udp套接字
参数:port 监听的端口号 
返回值: -1 创建失败 大于0的值创建成功
********************************************************/
int create_listen_udp(char *ipaddr,int port)
{
	struct sockaddr_in addr;
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	if(fd == -1)	
	{		
		perror("create udp socket fail!");
		return -1;	
	}	
  	const int on=1;		

	init_addr(&addr, ipaddr,  port);
 	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if(bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))<0)
	{
	  printf("initSvr: Udp socket bind error.\n");
	  return -1;
	}
	return fd;
}
/*******************************************************
函数功能:创建一个监听的udp套接字，设置超时返回状态
参数:port 监听的端口号 timeout 超时时间
返回值: -1 创建失败 大于0的值创建成功
********************************************************/
int create_listen_udp_timeout(char *ipaddr,int port,int timeout)
{
	int fd = create_listen_udp(ipaddr,port);
	set_sockfd_timeout(fd,timeout);
	return fd;
}

int create_client_brocast(struct sockaddr_in *broaddr,int port)
{
	int sockfd;
	const int on=1;
	char allHosts[] = "255.255.255.255";
	if((sockfd = socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		perror("create socket failed failed!");
		return -1;
	}
  	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0)
  	{
    		return -1;
  	}
	init_addr(broaddr, allHosts,  port);	
#if 0	
	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr))<0)
	{
	  	printf("bind broadcast failed\n");
	  	return -1;
	}
#endif
	return sockfd;
}	

int create_client_brocast_timeout(struct sockaddr_in *broaddr,int port)
{
	int sockfd = create_client_brocast(broaddr,port);
	set_sockfd_timeout(sockfd,2);
	return sockfd;
}
int create_listen_brocast(char *ipaddr,int port)
{
	struct sockaddr_in addr;
	const int on=1;
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	if(fd == -1)	
	{		
		perror("create udp socket fail!");
		return -1;	
	}
	init_addr(&addr, ipaddr,  port);	
	if(setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))<0)
  	{
    		printf("init broadcast setsockopt failed \n");
    		return -1;
  	}
	if (bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))<0)
	{
	  	printf("bind broadcast failed\n");
	  	return -1;
	}
	return fd;
}
