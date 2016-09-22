#include <stdio.h>
#include "../output/aes_demo.h"
#include "../output/queWorkMsg.h"


void handleMsg(const char *msg,int size)
{
	printf("msg = %s\n",msg);
}
void cleanMsg(const char *msg,int size)
{
	printf("cleanMsg(const char *msg,int size)\n");
}
queworklist *qlist;

int main(void)
{
	char aecout[100+16]; 
	char msg[100];
	int send_size = aes_en_char((const char *)msg,aecout,100);
	aes_de_char((const char *)msg,aecout,100);
	qlist = Init_WorkPthread(handleMsg);
	Add_WorkMsg(qlist,"ssssaslkascasvc",10);	
	Add_WorkMsg(qlist,"ssssaslkascasssssssssssssssssssss",10);	
//	while(1)
	{
		sleep(1);
	}
	Clean_WorkPthread(qlist,cleanMsg);
	printf("-----------------\n");	
	return 0;
}

