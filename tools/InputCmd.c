#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../pool/pool.h"


#define CMD_NAME "cmd :" 
typedef void pasreInputCmd(const char *inputCmd);

static void transfStr(char *dest, int flag)
{
  char *ptr;
  int len;
  
  ptr = dest;
  while (isspace(*ptr))	
    ptr++;

  len = strlen(ptr);
  if (ptr > dest)
    memmove(dest, ptr, len+1);

  
  ptr = dest+len-1;
  while (isspace(*ptr))
    ptr--;

  *(ptr+1) = '\0';

  ptr = dest;

  if (flag == 1)
    while (*ptr!='\0')
    {
      *ptr = tolower(*ptr);
      ptr++;
    }
}
static void *Interface(void *arg)
{
	char com[32];
	pasreInputCmd *call = (pasreInputCmd *)arg; 
	while(1)
	{
		printf(CMD_NAME);
    	fgets(com, sizeof(com), stdin);
    	transfStr(com, 1); 
		call((const char *)com);
	}
	return NULL;
}

static void signal_ctrl_c(int signo)
{
	switch (signo) 
	{
        case SIGINT:    //miss ctrl+c
		printf("Catch a signal -- ctrl+c  \n");
        	break;
        case SIGQUIT:
        	printf("Catch a signal -- ctrl+\\n");
        	break;
		case SIGTSTP:
        	printf("Catch a signal -- ctrl+z  \n");
    		break;
  	}
	return;
}

void init_interface(void pasreInputCmd(const char *inputCmd))
{
	signal(SIGINT, signal_ctrl_c);
	signal(SIGQUIT,signal_ctrl_c);
	signal(SIGTSTP,signal_ctrl_c);
	
	if(pthread_create_attr(Interface,pasreInputCmd)){
		perror("create failed");
		exit(-1);
	}
}

