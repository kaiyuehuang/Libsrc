#ifndef _TOOLS_H
#define _TOOLS_H

extern void init_interface(void pasreInputCmd(const char *inputCmd));

/*
@ �򿪴��� 
@ serial �����豸�ڵ�  speed���ô��ڲ�����
@  -1:��ʧ��   >0:�򿪵��׽���������
*/
extern int serial_open(char *serial,int speed);

#endif
