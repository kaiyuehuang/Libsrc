#ifndef _TOOLS_H
#define _TOOLS_H

extern void init_interface(void pasreInputCmd(const char *inputCmd));

/*
@ �򿪴��� 
@ serial �����豸�ڵ�  speed���ô��ڲ�����
@  -1:��ʧ��   >0:�򿪵��׽���������
*/
extern int serial_open(char *serial,int speed);

extern int u2g(char *inbuf,int inlen,char *outbuf,int outlen);

extern int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen);

#endif
