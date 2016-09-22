#ifndef _HEAD_MP3_H 
#define _HEAD_MP3_H

extern void get_mp3head(FILE *fp,unsigned short *rate,char *channel); 
extern int Getmp3Steamer(char *mp3data,int length,unsigned short *rate,char *channel);

#endif
