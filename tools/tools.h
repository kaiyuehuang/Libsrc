#ifndef _TOOLS_H
#define _TOOLS_H

extern void init_interface(void pasreInputCmd(const char *inputCmd));

/*
@ 打开串口 
@ serial 串口设备节点  speed设置串口波特率
@  -1:打开失败   >0:打开的套接字描述符
*/
extern int serial_open(char *serial,int speed);

#endif
