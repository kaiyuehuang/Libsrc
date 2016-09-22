#ifndef _MP3HEAD_H_  
#define _MP3HEAD_H_  
  
#include <stdio.h>  
#include <bits/types.h>

typedef __uint32_t uint32_t;
  
enum {  
    BITRATE_MPEG1,  
    BITRATE_MPEG2,  
    BITRATE_NUM  
};  
  
  
enum {  
    SAMPLERATE_MPEG1,  
    SAMPLERATE_MPEG2,  
    SAMPLERATE_MPEG25,  
    SAMPLERATE_NUM  
};  
  
#define MP3_FRAME_SYNC          0xFFE00000  
  
#define MASK_VERSION            0x00180000  
#define SHIFT_VERSION           19  
  
#define MPEG_1              3  
#define MPEG_2              2  
#define MPEG_25             0  
  
#define SHIFT_LAYER         17  
#define LAYER_3             1  
#define LAYER_2             2  
#define LAYER_1             3  
  
#define PROTECTION_BIT          0x00010000  
  
#define MASK_BITRATE            0x0000f000  
#define SHIFT_BITRATE           12  
  
  
#define MASK_SAMPLERATE         0x00000C00  
#define SHIFT_SAMPLERATE        10  
  
#define MASK_PADDING            0x00000200  
  
#define MASK_CHANNEL                0x000000C0  
#define CHANNEL_STEREO              0x00000000  
#define CHANNEL_JOINT               0x00000040  
#define CHANNEL_DUAL                0x00000080  

#define CHANNEL_MONO                0x000000C0  
  
struct mp3_frame {  
    uint32_t version;  
    uint32_t channel;  //通道
    int bitrate;  
    int samplingRate;	//采样率  
    int samples;  
    int size;  
    const unsigned char *data;  
};  
  
// 解析一帧mp3数据  
extern void get_mp3head(FILE *fp,int *rate,char *channel); 

/**************************************************
获取文件长度
***************************************************/
extern int get_file_len(FILE *fp);
  
extern char *STRSTR(const char *str, char needle);
  
#endif // _MP3_H_  

