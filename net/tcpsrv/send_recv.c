#include <stdio.h>
#include <sys/epoll.h>
#include  <netinet/in.h>    
#include  <arpa/inet.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "command.h"
#include "common/msgstruct.h"
#include "epoll.h"
#include "sql_sqlite.h"
#include "sock_fd.h"
#include "udpsrv/udp_server.h"
#include "udpsrv/udp_send.h"
#include "netconfig.h"
#include "pool/pthread_pool.h"
#include "pool/que_mempool.h"

#include "Typedef.h"

#include "debug.h"

#include "config.h"
#ifdef AES_128_DATA
#include "../aes128/aes_demo.h"
#endif

#ifdef SECLEC
#include <unistd.h>
#include <fcntl.h>
fd_set server_fd;
int max_fd;
#endif

#define DEBUG_TCP_MSG
#ifdef DEBUG_TCP_MSG  
#define TCP_MSG(fmt, args...) printf("send_recv: " fmt, ## args)  
#else   
#define TCP_MSG(fmt, args...) { }  
#endif  

/**********************************
本地服务器登录密码错误
***********************************/
#define PASSWD_ERR 		-5		//主机密码和序列号不对应
#define LIST_NOT_EXITS	-2  		//主机序列号不存在


/*************************************************
发送用户信息给p2p 服务器
*************************************************/
int send_demo_tcpmsg(char *buf,int size)
{
#ifdef AES_128_DATA
	char aecout[size+16]; 
	//对消息进行加密
	int send_size = aes_en_char((const char *)buf,aecout,size);
	if(send(conMsg.connect_fd,aecout,send_size,0)<=0)
	{
		perror("send_demo_tcpmsg  failed");
		return -1;
	}
#else
	if(send(conMsg.connect_fd,buf,size,0)<=0)
	{
		perror("send_demo_tcpmsg failed!");
		return -1;
	}
#endif
	return 0;
}

#define TEST_SEND_MSG			0xff
void test_send_msg_p2pserver(void)
{
	worklist msg;
	memset(&msg,0,WORK_LIST_SIZE);
	msg.msgtype = TEST_SEND_MSG;
	strcpy(msg.data.camlist,"I am localserver \n");
	send_demo_tcpmsg(&msg,WORK_LIST_SIZE);
	
}
	
/****************************************************************
sendto udp msg to net server to help finish p2p req
发送udp包给远程P2P服务器，响应app的请求
*****************************************************************/
static int handle_p2p_request(worklist *msg)
{
	int err=0;
	create_work_msg( msg, P2P_REQ_ACK, 0);
	//接收客户端请求p2p的序列号不对
	if(strcmp(msg->data.camlist,host.list))  
	{
		printf("host list diffrenct :");
		DEBUG_STR(LEVEL_1,msg->data.camlist);
		/************************************************
		通知p2p服务器更新本地主机的状态
		收到的序列号不对
		*************************************************/
		msg->head.error_msg=-1;
		if(send_help_p2pmsg((char *)msg,WORK_LIST_SIZE) == 0){
			printf("handle_p2p_req: sendto udp msg failed\n");
		}
		err =-1;
		return err;
	}
	
	// 发送数据到服务器,在自己的路由器里面打一个nat端口
	if(send_help_p2pmsg((char *)msg,WORK_LIST_SIZE) == 0)  
	{
		/************************************************
		保存客户端app打洞地址
		*************************************************/
		memcpy(&p2pMsg.app_addr,&msg->head.usrip,SOCK_ADDR_SIZE);  
		//sleep(2);
		 /***************************************
		 发送到 keep live 到APP，开始打洞
		 ****************************************/
		send_keep_live((char *)msg); 
		printf("sendto p2p req ack succese to server \n");
	}
	return err;
}

static void localserver_login_ack(worklist *msg,int sockfd)
{
	if(msg->head.error_msg ==0)
	{
		conMsg.connectState=SUCCESS_CONNECT;
		printf("login success\n");
	}
	else if(msg->head.error_msg ==PASSWD_ERR)	
	{
		printf("passwd error msg->head.error_msg=%d\n",msg->head.error_msg);
		conMsg.connectState=FAILED_CONNECT;
		close(sockfd);	
	}
	else if(msg->head.error_msg==LIST_NOT_EXITS)
	{
		printf("list not exits  msg->head.error_msg = %d\n",msg->head.error_msg);
		conMsg.connectState=NERVER_CONNECT;
		close(sockfd);	
	}	
}
static void handle_tcp_msg(int sockfd,char *recvbuf,int size)
{
#ifdef AES_128_DATA
	char output[size+16];
	if(aes_de_char((const char *)recvbuf,output,size))
	{
		memcpy(recvbuf,output,size+16);
	}else{
		memcpy(recvbuf,output,size);
	}
	worklist *msg = (worklist *)recvbuf;
#else
	worklist *msg = (worklist *)recvbuf;
#endif
	switch(msg->msgtype)
	{		
		case LOCAL_LOGIN_ACK:					//主机登录回应的数据包
			localserver_login_ack(msg,sockfd);
			break;

		case HOST_BIND_CAMLIST_ACK:				//用户绑定主机，并在服务器上注册的数据包
			if(msg->head.error_msg ==0)			//绑定成功，通知用户
			{
				conMsg.connectState=SUCCESS_CONNECT;
				update_sql_list_passwd(msg->data.camlist,msg->data.passwd);
				TCP_MSG(" login net server success  camlist =%s\n",msg->data.camlist);
			}
			else if(msg->head.error_msg==ERR_CAMLIST_EXIST)
			{
				TCP_MSG("bind camlist failed  ,camlist :%s is exist  \n",msg->data.camlist);
				conMsg.connectState=BIND_CAMLIST_EXIST;
			}else if(msg->head.error_msg==ERR_P2P_NOLINE)
			{
				conMsg.connectState=DATABASE_NOT_LINE;
			}
			tell_app_bindstate((char *)msg);
			break;
			
		case P2P_REQ:							//p2p请求打洞数据包
			handle_p2p_request(msg);
			break;	
			
		case HOST_KEEP_LIVE_ACK:					//心跳回应包
			conMsg.connectState=SUCCESS_CONNECT;
			TCP_MSG("recv host keep live ack\n");
			break;
			
		case P2P_REQ_ACK:
			TCP_MSG("p2p req ack msg !\n");	
			break;
		case LOCAL_FILE_DOWNLOAD:				//下载文件
			
			break;
		case UPDATE_VERSION:
			check_devices_version( sockfd,recvbuf, size);
			break;
			
		case TEST_SEND_MSG:
			printf("recv p2pserver msg : %s\n",msg->data.camlist);
			break;
		default:
			TCP_MSG("handle_tcp_msg have not type  %d\n",msg->msgtype);
			break;
		}
}
#ifdef EPOLL
void *handle_msg(void *arg) 
{    	
	int fd = *(int*)arg,size;
	char *recvbuf=NULL;
	queuelist *mem=NULL;
	while(1)
	{
		mem =(queuelist *)get_mem(&recvhead);
		recvbuf = mem->data;
		size = recv(fd, recvbuf, MEM_POOL_SIZE,0);//读取客户端socket流    
		if(size>0)
		{
			handle_tcp_msg(fd,recvbuf,size);
			free_mempool(&recvhead,(queuelist *)mem);
		}else{
			if(size == -1)
			{
				if (errno == EAGAIN)  //tcp 管道数据读完
				{
					perror("recv failed");
					break;
				}else if (errno == EINTR)
				{
					printf("recv EINTR\n");
					free_mempool(&recvhead,(queuelist *)mem);
					continue;
				}else{
					printf("recv othen \n");
					break;
				}
			}
			else if(size==0)
			{
				printf("disconnect longserver \n");		
				break;
			}
		}
	}
	free_mempool(&recvhead,(queuelist *)mem);
	return NULL;
}
#endif

/*******************************************************
函数功能: 检查用户是否绑定序列号，绑定则连接服务器
参数:		无
返回值:	0:已经绑定 -1 未绑定  
********************************************************/
static int check_camlist(void)
{
	if(!strcmp(host.list,FRIST_SMART_LIST))		
	{
		printf("usr not bind camlist\n ");
		return -1;
	}
	return 0;
}
/*******************************************************
函数功能: 检查用户是否绑定序列号，未绑定则连接服务器
参数:		无
返回值:	0:允许绑定 -1 已经绑定  -2 正在绑定
********************************************************/
int   check_usrbind_state(void)
{
	if(!check_camlist())
	{
		return -1;
	}
	if(conMsg.connectState==HOST_BIND_ING)		//主机正在绑定序列号
	{
		printf("error  host bind ing\n ");
		return -2;
	}
	conMsg.connectState=HOST_BIND_ING;
	return 0;
}
static int login_p2p_server(char *camlist,char *passwd,unsigned char msgtype)
{
	worklist msg ;
	if(strncmp(conMsg.netSrvip,"0.0.0.0",7)==0)
	{
		return -1;
	}
	if((conMsg.connect_fd = init_client_sock(conMsg.netSrvip,conMsg.port))<=0)
		return -1;
#ifdef EPOLL
	add_fd_epoll(conMsg.connect_fd,epollmsg);
#elif SECLEC
	if (setnonblocking(conMsg.connect_fd) < 0) 
	{        
		perror("setnonblock error");    
	} 
#endif
	/** send local host login msg to net server **/
	 init_work_msg(&msg,msgtype,0);
	strcpy(msg.data.camlist,camlist);
	strcpy(msg.data.passwd,passwd);
	if(send_demo_tcpmsg((char *)&msg,WORK_LIST_SIZE)<0)
	{
		return -1;
	}
	return conMsg.connect_fd;
}
/*******************************************************
函数功能:  绑定设备序列号
参数:		camlist :序列号  passwd :设备端密码
返回值:	大于0的值:连接成功 -1 连接失败
********************************************************/
int start_bind_camlist(char *camlist,char *passwd)
{
	return login_p2p_server(camlist,passwd,HOST_BIND_CAMLIST);
}

/*******************************************************
函数功能: 本地设备连接服务器
参数:		camlist :序列号  passwd :设备端密码
返回值:	大于0的值:连接成功 -1 连接失败
********************************************************/
int start_login_p2pserver(char *camlist,char *passwd)
{
	return login_p2p_server(camlist,passwd,LOCAL_LOGIN);
}
int client_recv(void)
{
	int size;
	queuelist *mem=NULL;
	mem = (queuelist *)get_mem(&recvhead);
//	start_bind_camlist("test_camlist123",FRIST_PASSWD);
	while(1)
	{
		if((size = recv(conMsg.connect_fd, mem->data, MEM_POOL_SIZE,0))>0)//读取客户端socket流	  
		{
			handle_tcp_msg(conMsg.connect_fd,mem->data,size);
			continue;
		}
		if(check_camlist())
		{
			//printf("wait usr bind camlist and connect server ...... conMsg.connectState = %d\n",conMsg.connectState);
			if(conMsg.connectState!=HOST_BIND_ING)
				conMsg.connectState  =WAIT_USR_BIND;
			sleep(3);
			continue;
		}
		sleep(5);

		if(conMsg.connectState==NERVER_CONNECT)	//序列号不存在，禁止连接服务器
		{
			printf("camlist not exist ,disable permission connect server\n");
			continue;		
		}
		if(conMsg.connect_fd>2)
			close(conMsg.connect_fd);	

		printf("disconnect p2p server  \n"); 
		conMsg.connectState=DISCONNECT;
		start_login_p2pserver(host.list,host.passwd);
	}
	free_mempool(&recvhead,(queuelist *)mem);
	return 0;
}
