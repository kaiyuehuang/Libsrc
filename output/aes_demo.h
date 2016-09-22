#ifndef _AES_DEMO_H
#define _AES_DEMO_H

/*******************************************************
函数功能: 对数据进行加密 
参数: src 原始数据  dest加密之后的数据 srclen原始数据长度
返回值: 加密之后的数据长度
********************************************************/
extern int aes_en_char(const char *src,char *dest,int srclen);

/*******************************************************
函数功能: 对数据进行解密 
参数: src 加密数据  dest解密之后得到的原始数据 srclen加密数据长度
返回值: 解密之后的数据长度
********************************************************/
extern int aes_de_char(const char *src,char *dest,int srclen);
	
#endif
