#ifndef _AES_DEMO_H
#define _AES_DEMO_H

/*******************************************************
��������: �����ݽ��м��� 
����: src ԭʼ����  dest����֮������� srclenԭʼ���ݳ���
����ֵ: ����֮������ݳ���
********************************************************/
extern int aes_en_char(const char *src,char *dest,int srclen,int *outLen);

/*******************************************************
��������: �����ݽ��н��� 
����: src ��������  dest����֮��õ���ԭʼ���� srclen�������ݳ���
����ֵ: ����֮������ݳ���
********************************************************/
extern int aes_de_char(const char *src,char *dest,int srclen,int *outLen);

extern void setKey(char *key);
	
#endif
