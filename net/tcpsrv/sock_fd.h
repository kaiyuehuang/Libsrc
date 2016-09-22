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

#endif
