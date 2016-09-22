#include "mp3head.h"  
  
  
// bit率   128K bit/s    
static int bitrateTbl[16][BITRATE_NUM] =  
{  
    { -1,  -1},  
    { 32,   8},  
    { 40,  16},  
    { 48,  24},  
    { 56,  32},  
    { 64,  40},  
    { 80,  48},  
    { 96,  56},  
    {112,  64},  
    {128,  80},  
    {160,  96},  
    {192, 112},  
    {224, 128},  
    {256, 144},  
    {320, 160},  
    { -1,  -1}  
};  
  
// 采样率  
static int samplingRateTbl[3][SAMPLERATE_NUM] =  
{  
    { 44100, 22050, 11025 },  
    { 48000, 24000, 12000 },  
    { 32000, 16000,  8000 }  
};  
  
// 检查帧头信息  
static int checkHdr(uint32_t hdr)  
{  
    if( (hdr & MP3_FRAME_SYNC) != MP3_FRAME_SYNC  
        || ((hdr>>SHIFT_VERSION) & 0x3) == 1  
        || ((hdr>>SHIFT_LAYER) & 0x3) == 0  
        || ((hdr>>SHIFT_BITRATE) & 0xf) == 0xf   
        || ((hdr>>SHIFT_BITRATE) & 0xf) == 0  
        || ((hdr>>SHIFT_SAMPLERATE)&0x3) == 0x3  
        || (hdr & 0xffff0000) == 0xfffe0000)  
        return 0;  
    return 1;  
}  
  
#define READ_ERROR(ret) if((ret) < 0) return (ret);  
  
static int READ_CHAR(FILE * fp, uint32_t * v)  
{  
    char buf[1];  
  
    if(fread(buf, 1, 1, fp) <= 0)  
    {
        return -1; 
        }
  
    *v = 0xff & buf[0];  
    return 0;  
}  
  
static int READ_UINT16(FILE * fp, uint32_t * v)  
{  
    uint32_t tmp;  
    int ret;  
  
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v = (tmp << 8);  
  
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v |= (tmp);  
  
    return 0;  
}  
  
static int READ_UINT24(FILE * fp, uint32_t * v)  
{  
    uint32_t tmp;  
    int ret;  
  
    ret = READ_UINT16(fp, &tmp);  
    READ_ERROR(ret);  
    *v = (tmp << 8);  
      
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v |= (tmp);  
  
    return 0;  
}  
  
static int READ_UINT32(FILE * fp, uint32_t * v)  
{  
    uint32_t tmp;  
    int ret;  
  
    ret = READ_UINT24(fp, &tmp);  
    READ_ERROR(ret);  
    *v = (tmp << 8);  
      
    ret = READ_CHAR(fp, &tmp);  
    READ_ERROR(ret);  
    *v |= (tmp);  
  
    return 0;  
}                                     
  
/***********************************************************
解析mp3帧，获取mp3头信息 
***********************************************************/ 
static int getNextFrame(FILE * mp3fp, struct mp3_frame * frame)  
{  
    uint32_t hdr, br_idx, sr_idx;     
    int ret;  
    int factor;   
    do  
    {  
        ret = READ_UINT32(mp3fp, &hdr);  
        //ret = fread(&hdr, 4, 1, mp3fp);  
        if (ret < 0)  
            return -1;  
        if (checkHdr(hdr))  
            break;  
          
        fseek(mp3fp, -3, 1);  
    }while (1);  
      
    frame->version = (hdr >> SHIFT_VERSION) & 0x3;  
    br_idx = (hdr >> SHIFT_BITRATE) & 0xf;  
    sr_idx = (hdr >> SHIFT_SAMPLERATE) & 0x3;  
  
    switch(frame->version)  
    {  
    case MPEG_25:  
        frame->bitrate = bitrateTbl[br_idx][BITRATE_MPEG2];  
        frame->samplingRate =   
            samplingRateTbl[sr_idx][SAMPLERATE_MPEG25];  
        factor = 72;  
        break;  
    case MPEG_2:  
        frame->bitrate = bitrateTbl[br_idx][BITRATE_MPEG2];  
        frame->samplingRate =   
            samplingRateTbl[sr_idx][SAMPLERATE_MPEG2];  
        factor = 72;  
        break;  
    case MPEG_1:  
        frame->bitrate = bitrateTbl[br_idx][BITRATE_MPEG1];  
        frame->samplingRate =   
            samplingRateTbl[sr_idx][SAMPLERATE_MPEG1];  
        factor = 144;  
        break;  
    default:  
        printf("invalid header %x\n", hdr);  
        return -1;  
    }  
  
    frame->size = (factor * frame->bitrate * 1000) / frame->samplingRate;    
    if(frame->size == 0)  
    {  
        printf("size == 0\n");  
        return -1;  
    }  
  
    if((hdr & MASK_PADDING) != 0)  
        frame->size++;  
  
    if(frame->samplingRate > 32000)  
        frame->samples = 1152;  
    else  
        frame->samples = 576;  
  
    frame->channel = hdr & MASK_CHANNEL;  
    frame->channel = frame->channel>>6;
  
    fseek(mp3fp, -4, 1);  
  
    return 0;  
}  

/*****************************************************
获取采样率和通道数rate:采样率channel:通道
*****************************************************/
void get_mp3head(FILE *fp,int *rate,char *channel)
{
	if(fp ==NULL)
	{
		return ;
	}
	struct mp3_frame pframe;
    getNextFrame(fp,&pframe);    
    printf("This song samplingRate is %dHz\n",pframe.samplingRate);
	*rate = pframe.samplingRate;
    if(pframe.channel == 0x03)
    {
		printf("This song channel is 1 channel\n");
		*channel = 1;
    }
    else
    {
        printf("This song channel is 2 channel\n");
		*channel = 2;
	}
}
int get_file_len(FILE *fp)
{
	int fsta,fend,cur_pos;
	cur_pos = ftell(fp);

	fseek(fp, 0, SEEK_SET);//跳转到头部
	fsta = ftell(fp); 
	fseek(fp, 0, SEEK_END);//跳转到尾部
    fend = ftell(fp);

	fseek(fp, cur_pos, SEEK_SET);
	printf("fend=%d  fsta=%d \n",fend,fsta);
    return  fend - fsta;
}
/********************************************************
从歌曲的绝对路径提取出具体的文件名字
/mnt/sdcard/test.mp3  ------>test.mp3
********************************************************/
char *STRSTR(const char *str, char needle)
{
	char *dest=NULL;
	if(str==NULL)
		return NULL;
	int i,len = strlen(str);
	for(i=len; i>0; i--)
	{
        if(*(str+i-1)==needle)
        	break;
	}
	if(i==0)
		return NULL;
	dest = (char *)str+i;
	return dest;
}
