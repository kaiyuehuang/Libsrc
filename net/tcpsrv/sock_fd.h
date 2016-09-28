#ifndef _SOCK_FD_H
#define _SOCK_FD_H

#define LISTENMAX 	5

extern int gethost_ip(char *ip,char *hostname);

extern int get_serverip(char *ip,char *hostname);

#ifdef CONFIG_CLIENT
extern int create_client(char *serverip,int port);
#endif

#ifdef CONFIG_SERVER
extern int create_server(char *bindip,int port);
#endif

extern int GetsockRecvSize(int sock,int *size);
extern int GetsockSendSize(int sock,int *size);
extern int SetsockRecvSize(int sock,int size);
extern int SetsockSendSize(int sock,int size);
extern int Setnoblock(int sockfd,int blocking);
extern int SetTcpNoDelay(int sockfd);

#endif
