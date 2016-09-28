#ifndef _HEAD_MP3_H 
#define _HEAD_MP3_H

typedef struct{
	unsigned short rate;
	unsigned char channel;
	int samplingRate;
	int bitrate;
}Mp3_t;
  
extern Mp3_t *get_mp3head(FILE *fp); 
extern Mp3_t *Getmp3Steamer(char *mp3data,int length);
extern int getmp3Toalltime(int bitrate,int fileLen);

#endif
