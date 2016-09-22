#ifndef _MADPLAY_H
#define _MADPLAY_H

#define BUFSIZE 			8192//mp3一帧最大值

#define MAD_PLAY_NOW 		20	//播放
#define MAD_PAUSEE			21 	//暂停
#define MAD_EXIT_PLAY		22	//停止	
#define NET_UPDATE_FILE		23	//网络更新文件
#define MAD_NEXT_MUISC		25	//下一首

/**********************************************
开始播放音乐
**********************************************/
extern void decode_start_music(void);

/**********************************************
暂停播放音乐
**********************************************/
extern void decode_stop_music(void);

/**********************************************
退出播放音乐
**********************************************/
extern void decode_exit_music(void);

/**********************************************
退出切换到下一首音乐
**********************************************/
extern void decode_next_music(void);
/**********************************************
获取当前播放状态，主要用来切换采样率
**********************************************/
extern unsigned char get_decode_music_state(void);

/**********************************************
开启播放网络音乐
**********************************************/
extern int decode_net_music(char *music_src,int file_len);

/**********************************************
初始化解码播放器
**********************************************/
extern void init_decode_music(void);

extern void clean_decode_music(void);

#endif
