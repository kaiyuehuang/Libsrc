#ifndef _MADPLAY_H
#define _MADPLAY_H

#define BUFSIZE 			8192//mp3һ֡���ֵ

#define MAD_PLAY_NOW 		20	//����
#define MAD_PAUSEE			21 	//��ͣ
#define MAD_EXIT_PLAY		22	//ֹͣ	
#define NET_UPDATE_FILE		23	//��������ļ�
#define MAD_NEXT_MUISC		25	//��һ��

/**********************************************
��ʼ��������
**********************************************/
extern void decode_start_music(void);

/**********************************************
��ͣ��������
**********************************************/
extern void decode_stop_music(void);

/**********************************************
�˳���������
**********************************************/
extern void decode_exit_music(void);

/**********************************************
�˳��л�����һ������
**********************************************/
extern void decode_next_music(void);
/**********************************************
��ȡ��ǰ����״̬����Ҫ�����л�������
**********************************************/
extern unsigned char get_decode_music_state(void);

/**********************************************
����������������
**********************************************/
extern int decode_net_music(char *music_src,int file_len);

/**********************************************
��ʼ�����벥����
**********************************************/
extern void init_decode_music(void);

extern void clean_decode_music(void);

#endif
