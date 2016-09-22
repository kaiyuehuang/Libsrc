#ifndef _QUEWORKMSG_H_
#define _QUEWORKMSG_H_

#include <pthread.h>
#include <semaphore.h>

typedef struct queque_list
{      
	int msgSize;
	const char *msg;
	struct queque_list *next;
}Workudp;

typedef struct que_worklist
{
	unsigned char pthread_live;
	unsigned short listNum;
	Workudp workhead;	
	Workudp *worktail;
	pthread_mutex_t workmutex;
	sem_t Empty;
	sem_t Full;
	void (*callFuntion)(const char *msg,int msgSize);
}queworklist;

extern int get_WorkMsgNum(queworklist *queList);

extern int Add_WorkMsg(queworklist *queList,const char *msg,int msgSize);

extern queworklist *Init_WorkPthread(void handleMsg(const char *msg,int msgSize));

extern void Clean_WorkPthread(queworklist *queList,void cleanMsg(const char *msg,int msgSize));

#endif
