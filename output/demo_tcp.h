#ifndef _DEMO_TCP_H
#define _DEMO_TCP_H

#include <netdb.h>

/*******************************************************
��������: ����һ��tcp���ӿͻ���
����: serverip ��������ip��ַ port �����������Ķ˿�
����ֵ: -1����ʧ�� ����0��ֵ���ӳɹ�֮��
********************************************************/
extern int create_client(char *serverip,int port);

/*******************************************************
��������: ����һ��tcp����������
����: bindip�󶨵�IP ��ַ port�����Ķ˿ں�
����ֵ: -1����ʧ�� ����0��ֵ�����ɹ�
********************************************************/
extern int create_server(char *bindip,int port);

extern int GetsockRecvSize(int sock,int *size);
extern int GetsockSendSize(int sock,int *size);
extern int SetsockRecvSize(int sock,int size);
extern int SetsockSendSize(int sock,int size);
extern int Setnoblock(int sockfd,int blocking);
extern int SetTcpNoDelay(int sockfd);

extern struct hostent *timeGesthostbyname(char *hostname,int timeout);
extern int checkNetwork(int timeout);
//��ȡ������IP��ַ
extern int GetNetworkcardIp(char *NetName,char *IP);
//��ȡ��ǰ���������IP��ַ
extern int getwlanip(char *wlanip);

#endif
