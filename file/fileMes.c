#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int GetFileLen(FILE *fp)
{
	int fsta,fend,cur_pos;
	cur_pos = ftell(fp);

	fseek(fp, 0, SEEK_SET);//Ìø×ªµ½Í·²¿
	fsta = ftell(fp); 
	fseek(fp, 0, SEEK_END);//Ìø×ªµ½Î²²¿
    	fend = ftell(fp);

	fseek(fp, cur_pos, SEEK_SET);
	//printf("fend=%d  fsta=%d \n",fend,fsta);
    	return  fend - fsta;
}
/********************************************************
´Ó¸èÇúµÄ¾ø¶ÔÂ·¾¶ÌáÈ¡³ö¾ßÌåµÄÎÄ¼þÃû×Ö
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

static int __readFileDir(char *basePath,void callfilename(char *filepath,char *name))
{
	DIR *dir;
    struct dirent *ptr;
	struct stat buf;
	char path[1024]={0};
	if ((dir=opendir(basePath)) == NULL)
   	{
       		perror("Open dir error...");
       		return -1;
   	}
   	while ((ptr=readdir(dir)) != NULL)
   	{
    		//printf("d_name:%s %s\n",basePath,ptr->d_name);
		if((!strcmp(ptr->d_name,"."))||(!strcmp(ptr->d_name,"..")))
		{
			continue;
		}
   		snprintf(path,1024,"%s%c%s",basePath,'/',ptr->d_name);
		//printf("path = %s\n",path);
		if(lstat(path,&buf)<0)
		{
			perror("lstat error");
			break;
		}
		if (S_ISREG(buf.st_mode))
		{
			//callfilename(ptr->d_name);
			callfilename(path,ptr->d_name);
		}
		else if(S_ISDIR(buf.st_mode))
		{
   			snprintf(path,1024,"%s%c%s",basePath,'/',ptr->d_name);
			__readFileDir(path,callfilename);
		}
  	}
     closedir(dir);
     return 0;
 }
int ScanDir(char *basePath,void callfilename(char *filepath,char *name))
{
	if(basePath==NULL)
		return -1;
	if(access(basePath,F_OK))
	{
		printf("have not dir \n");
		return -1;	
	}	
	int len = strlen(basePath);
	if(basePath[len-1]=='/')
	{
		basePath[len-1]='\0';
	}
	return __readFileDir(basePath,callfilename);	
}
#if 0
void test_read(char *filepath,char *name)
{
	printf("%s%s%s\n",filepath,"--->",name);
}

int main(int argc,char **argv)
{
	ScanDir(argv[1],test_read);
	return 0;
}
#endif
