#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>	
#include <sys/socket.h>
#include <net/if.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <setjmp.h>
#include<unistd.h>
#include <netdb.h>

static sigjmp_buf jmpbuf;
static void alarm_func(int sig)
{
	printf("time out\n");
	siglongjmp(jmpbuf,1);
}

struct hostent *timeGesthostbyname(char *hostname,int timeout)
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
int checkNetwork(int timeout)
{
	if(timeGesthostbyname("www.baidu.com",timeout)==NULL)
	{
		return -1;
	}
	return 0;
}
/*******************************************************
函数功能:根据网卡名字获取网卡的IP地址
参数:NetName网卡名字 IP获取出来的ip地址
返回值: 0 获取成功 -1 获取失败
********************************************************/
int GetNetworkcardIp(char *NetName,char *IP)
{
	struct sockaddr_in server_addr;
	struct ifreq ifr;
	strncpy(ifr.ifr_name, NetName, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	if(fd == -1)	
	{		
		perror("get_networkcard_ip : create udp socket fail!");
		return -1;	
	}
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		perror("get_networkcard_ip : ioctl error\n");
        return -1;
    }
	memcpy(&server_addr, &ifr.ifr_addr, sizeof(server_addr));
   	sprintf(IP,"%s", (char *)inet_ntoa(server_addr.sin_addr));
	if(!memcmp(IP,"0.0.0.0",7))  //是否拿到正确的IP
	{
		return -1;
	}
	printf(" (char *)inet_ntoa(server_addr.sin_addr) = %s\n", (char *)inet_ntoa(server_addr.sin_addr));
	close(fd);
   	return 0;
}

