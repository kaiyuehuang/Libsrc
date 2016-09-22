#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "mad.h"
#include "madplay.h"
#include "mp3head.h"

//同一个位置解码出错超过30次。表明为无效音频
#define DECODE_ERROR_NUM	60 

//#define DEBUG_MAD
#ifdef DEBUG_MAD  
#define DEBUG_MADPLAY(fmt, args...) printf("madplay mp3 music: " fmt, ## args)  
#else   
#define DEBUG_MADPLAY(fmt, args...) { }  
#endif  

#ifdef DEBUG_MAD  
#define DEBUG_MADPLAY_ERR(fmt, args...) printf("madplay mp3 music: " fmt, ## args)
#else   
#define DEBUG_MADPLAY_ERR(fmt, args...) { }  
#endif  

//#define X86_PLAY
//#define	TEST_PLAY_MODE 

#ifdef X86_PLAY
#include <sys/soundcard.h>
#else
#include "init_i2s.h"
#endif
/*********************************************
一边解码一边播放
**********************************************/
#define FILE_LEN 	128
/*
* This is a private message structure. A generic pointer to this structure
* is passed to each of the callback functions. Put here any data you need
* to access from within the callbacks.
*/   
typedef struct buffer {
	unsigned short error_time;		//解码出错时间
	unsigned char err_tices;		//解码出错次数
	
	unsigned char playstate;		//播放状态
	unsigned short fbsize; 			/*indeed size of buffer*/
    FILE *fp; 						/*file pointer*/
    unsigned int flen; 				/*file length*/
    unsigned int fpos; 				/*current position*/
    unsigned char fbuf[BUFSIZE]; 	/*buffer*/
	char filename[FILE_LEN];				//文件路径
}mp3_file;
static mp3_file Mp3play;


#ifdef X86_PLAY
int soundfd; /*soundcard file*/
unsigned int prerate = 0; /*the pre simple rate*/
static int writedsp(int c)
{
    return write(soundfd, (char *)&c, 1);
}
static void set_dsp(void)
{
    int rate = 44100;
  	int format = AFMT_S16_LE;
    int channels = 2;
    int value;
    soundfd = open("/dev/dsp", O_WRONLY);
                                                                                                                                                                                                                                                                                                                                                                                            
    ioctl(soundfd,SNDCTL_DSP_SPEED,&rate);
    ioctl(soundfd, SNDCTL_DSP_SETFMT, &format);
    ioctl(soundfd, SNDCTL_DSP_CHANNELS, &channels);
/*
    value = 16;
    ioctl(soundfd,SNDCTL_DSP_SAMPLESIZE,&value);
    value = 0;
    ioctl(soundfd,SNDCTL_DSP_STEREO,&value);
*/
}
#endif
/*
* This is perhaps the simplest example use of the MAD high-level API.
* Standard input is mapped into memory via mmap(), then the high-level API
* is invoked with three callbacks: input, output, and error. The output
* callback converts MAD's high-resolution PCM samples to 16 bits, then
* writes them to standard output in little-endian, stereo-interleaved
* format.
*/

/*
* This is the input callback. The purpose of this callback is to (re)fill
* the stream buffer which is to be decoded. In this example, an entire file
* has been mapped into memory, so we just call mad_stream_buffer() with the
* address and length of the mapping. When this callback is called a second
* time, we are finished decoding.
*/
#if 0
static void auto_set_vol(void)
{
	i2s_add_sub_vol(1);
}
#endif
static enum mad_flow input(void *data,struct mad_stream *stream)
{
    mp3_file *mp3fp;
    int ret_code;
    int unproc_data_size; /*the unprocessed data's size*/
    int copy_size;
    mp3fp = (mp3_file *)data;
	DEBUG_MADPLAY("mp3fp->playstate = %d mp3fp->fpos = %d,mp3fp->flen = %d\n",Mp3play.playstate,mp3fp->fpos,mp3fp->flen);
	if(Mp3play.playstate==MAD_EXIT_PLAY)
	{
		 ret_code = MAD_FLOW_STOP;
	}
	else if(Mp3play.playstate==MAD_PAUSEE)
	{	
		DEBUG_MADPLAY("Mp3play.playstate is stop\n");
		sleep(1);
	}
	else if(Mp3play.playstate==MAD_NEXT_MUISC)
	{
		DEBUG_MADPLAY("Mp3play.playstate is NEXT_MUISC\n");
		ret_code = MAD_FLOW_STOP;
	}
	else
	{
    	if(Mp3play.fpos <= Mp3play.flen-1)
    	{	
    	    		fclose(Mp3play.fp);
			Mp3play.fp =NULL;
			if((Mp3play.fp = fopen(Mp3play.filename, "r"))!=NULL)
			{
				fseek(Mp3play.fp,Mp3play.fpos,SEEK_SET);
    				/****************************************************
    				mp3根据每一完整的帧才能解码出来
    				*****************************************************/
        		unproc_data_size = stream->bufend - stream->next_frame;
        		memcpy(Mp3play.fbuf, Mp3play.fbuf+Mp3play.fbsize-unproc_data_size, unproc_data_size);
        		copy_size = BUFSIZE - unproc_data_size;
        		if(Mp3play.fpos + copy_size > Mp3play.flen)
        		{
            			copy_size = Mp3play.flen - Mp3play.fpos;
        		}	
        		fread(Mp3play.fbuf+unproc_data_size, 1, copy_size, Mp3play.fp);
        		Mp3play.fbsize = unproc_data_size + copy_size;
       			Mp3play.fpos += copy_size;
        		/*Hand off the buffer to the mp3 input stream*/
        		mad_stream_buffer(stream, Mp3play.fbuf, Mp3play.fbsize);
        		ret_code = MAD_FLOW_CONTINUE;

				//if(Mp3play.fpos<BUFSIZE*8)
					//auto_set_vol();
			}
    	}
		else
		{
			DEBUG_MADPLAY("----------------MAD_FLOW_STOP--------------------------\n");
			ret_code = MAD_FLOW_STOP;
			Mp3play.playstate=MAD_EXIT_PLAY;
		}
	}
    return ret_code;
}
/*
* The following utility routine performs simple rounding, clipping, and
* scaling of MAD's high-resolution samples down to 16 bits. It does not
* perform any dithering or noise shaping, which would be recommended to
* obtain any exceptional audio quality. It is therefore not recommended to
* use this routine if high-quality output is desired.
*/
static inline signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L <= MAD_F_FRACBITS - 16);
    if(sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if(sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}
/*
* This is the output callback function. It is called after each frame of
* MPEG audio data has been completely decoded. The purpose of this callback
* is to output (or play) the decoded PCM audio.
*/
#ifndef X86_PLAY	
static unsigned short play_size=0;
#endif
static enum mad_flow output(void *data,struct mad_header const *header,struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples;
    unsigned int rate;
	signed int sample;
    mad_fixed_t const *left_ch, *right_ch;
	char buf[4];
    /* pcm->samplerate contains the sampling frequency */
    rate = pcm->samplerate;
    nchannels = pcm->channels;
    nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];
    /* update the sample rate of dsp*/
#ifdef X86_PLAY	
    if(rate != prerate)
    {
        ioctl(soundfd, SNDCTL_DSP_SPEED, &rate);
        prerate = rate;
    }
#endif
    while (nsamples--)
    {
        /* output sample(s) in 16-bit signed little-endian PCM */
        sample = scale(*left_ch++);
#ifdef X86_PLAY
		writedsp((sample >> 0) & 0xff);
      	writedsp((sample >> 8) & 0xff);
#else
		buf[0]=(sample >> 0) & 0xff;
		buf[1]=(sample >> 8) & 0xff;
		memcpy(play_buf+play_size,buf,2);
		play_size +=2;
#endif
		if (nchannels == 2)
        {
            sample = scale(*right_ch++);
#ifdef X86_PLAY
			writedsp((sample >> 0) & 0xff);
            writedsp((sample >> 8) & 0xff);
#else
			buf[2]=(sample >> 0) & 0xff;
			buf[3]=(sample >> 8) & 0xff;
			memcpy(play_buf+play_size,buf,2);
			play_size +=2;
			if(play_size==I2S_PAGE_SIZE)
			{
				play_size=0;
				write_pcm(play_buf);
			}
#endif
		}
    }
    return MAD_FLOW_CONTINUE;
}
/*
* This is the error callback function. It is called whenever a decoding
* error occurs. The error is indicated by stream->error; the list of
* possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
* header file.
*/
static enum mad_flow error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{
    mp3_file *mp3fp = data;
    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
    stream->error, mad_stream_errorstr(stream),
    stream->this_frame - mp3fp->fbuf);

	if(Mp3play.err_tices++>DECODE_ERROR_NUM)
	{
		Mp3play.err_tices=0;
		decode_exit_music();
	}
    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
    return MAD_FLOW_CONTINUE;
}
/*
* This is the function called by main() above to perform all the decoding.
* It instantiates a decoder object and configures it with the input,
* output, and error callback functions above. A single call to
* mad_decoder_run() continues until a callback function returns
* MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
* signal an error).
*/
static int decode(mp3_file *mp3fp)
{
    struct mad_decoder decoder;
    int result;
    /* configure input, output, and error functions */
    mad_decoder_init(&decoder, mp3fp,
    input, 0 /* header */, 0 /* filter */, output,
    error, 0 /* message */);
    /* start decoding */
    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
    /* release the decoder */
    mad_decoder_finish(&decoder);
    return result;
}
static void set_music_state(unsigned char playstate)
{
	if(Mp3play.playstate==0||Mp3play.playstate==MAD_EXIT_PLAY)
	{
		DEBUG_MADPLAY("music no play\n");
		return;
	}
	Mp3play.playstate = playstate;
}	

/**********************************************
开始播放音乐
**********************************************/
void decode_start_music(void)
{
	Mp3play.playstate=MAD_PLAY_NOW;
}

/**********************************************
暂停播放音乐
**********************************************/
void decode_stop_music(void)
{
	set_music_state(MAD_PAUSEE);
}
/**********************************************
退出播放音乐
**********************************************/
void decode_exit_music(void)
{
	set_music_state(MAD_EXIT_PLAY);
}

void decode_next_music(void)
{
	Mp3play.playstate=MAD_NEXT_MUISC;
}
/*******************************************
获取当前播放状态，主要用来切换采样率
********************************************/
unsigned char get_decode_music_state(void)
{
	return Mp3play.playstate;
}
#ifdef X86_PLAY
int main(int argc, char *argv[])
{
    long flen, fsta, fend;
    int dlen;
    if (argc < 2)
   		return 1;
    if((Mp3play.fp = fopen(argv[1], "r")) == NULL)
    {
        printf("can't open source file.\n");
        return 2;
    }
	memcpy(Mp3play.filename,argv[1],strlen(argv[1]));
	
    fsta = ftell(Mp3play.fp);
    fseek(Mp3play.fp, 0, SEEK_END);
    fend = ftell(Mp3play.fp);
    flen = fend - fsta;
    fseek(Mp3play.fp, 0, SEEK_SET);
    fread(Mp3play.fbuf, 1, BUFSIZE, Mp3play.fp);
    Mp3play.fbsize = BUFSIZE;
    Mp3play.fpos = BUFSIZE;
   	Mp3play.flen = flen;
	Mp3play.playstate = MAD_PLAY_NOW;
		
    set_dsp();
    decode(&Mp3play);
    close(soundfd);
    fclose(Mp3play.fp);
    return 0;
}
#else

/****************************************************************************************
music_src:网络播放文件名字
file_len:文件大小
(网络接收过来的文件先缓冲一段音频文件，
然后在input里面回调每次都需要打开和关闭才能定位文件大小)
*****************************************************************************************/
int decode_net_music(char *music_src,int file_len)
{
//	DEBUG_MADPLAY("decode_net_music = %s\n",music_src);
	printf("decode_net_music = %s\n",music_src);

	/****************************************************
	保存当前播放的音乐
	*****************************************************/
	memcpy(Mp3play.filename,music_src,strlen(music_src));
	usleep(3000);
    if((Mp3play.fp = fopen(Mp3play.filename, "r")) == NULL)
    {
       	DEBUG_MADPLAY_ERR("can't open source file. Mp3play filename= %s\n",Mp3play.filename);
       	return -1;
    }	
#if 0
	int rate;
	char ch;
	get_mp3head(Mp3play.fp,&rate,&ch);
	//fseek(Mp3play.fp, 0, SEEK_END);
	Mp3play.flen = ftell(Mp3play.fp);
	printf("file len = %d\n",Mp3play.flen);
#else	
	Mp3play.flen = file_len;
#endif	

    fseek(Mp3play.fp, 0, SEEK_SET);
    fread(Mp3play.fbuf, 1, BUFSIZE, Mp3play.fp);
    Mp3play.fbsize = BUFSIZE;
    Mp3play.fpos = BUFSIZE;

	Mp3play.error_time=0;
	Mp3play.err_tices=0;
	
 
    decode(&Mp3play);

	if(Mp3play.fp!=NULL)
		fclose(Mp3play.fp);

	DEBUG_MADPLAY("decode_net_music exit Mp3play.playstate = %d\n",Mp3play.playstate);
	
	memset(Mp3play.filename,0,FILE_LEN);
		
	return Mp3play.playstate;
}



/*******************************************
初始化解码结构体
********************************************/
void init_decode_music(void)
{
	memset(&Mp3play,0,sizeof(mp3_file));

}
/*******************************************
清除解码器，退出播放
********************************************/
void clean_decode_music(void)
{
	Mp3play.playstate =MAD_EXIT_PLAY;
	usleep(1000);
}
#endif
