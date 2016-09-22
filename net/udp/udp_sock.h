#ifndef _SOCK_UDP_H
#define _SOCK_UDP_H

#include <netinet/in.h>

#define SOCK_ADDR_SIZE sizeof(struct sockaddr_in)

extern void init_addr(struct sockaddr_in *addr, char *ip,  int port);

extern int get_networkcard_ip(char *net_name,char *IP);

extern int create_listen_udp_timeout(char *ipaddr,int port,int timeout);

extern int create_listen_udp(char *ipaddr,int port);

extern int create_client_brocast(struct sockaddr_in *broaddr,int port);

extern int create_client_brocast_timeout(struct sockaddr_in *broaddr,int port);

extern int create_listen_brocast(char *ipaddr,int port);

#endif

