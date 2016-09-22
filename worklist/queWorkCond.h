#ifndef _QUEWORKCOND_H
#define _QUEWORKCOND_H

#include <pthread.h>
#define WORKMSG_SIZE sizeof(WorkMsg)
typedef struct workmsg{      
	int msgSize;
	const char *msg;
	struct workmsg *next;
}WorkMsg;

typedef struct {
  int cacheWorkSize;
  WorkMsg workhead, *worktail;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  unsigned char pthread_live;	
  void (*callFuntion)(const char *msg,int msgSize);
} WorkQueue;

extern int getWorkMsgNum(WorkQueue *queList);
extern WorkQueue *initQueue(void);
extern int putMsgQueue(WorkQueue *queList,const char *msg,int msgSize);
extern int getMsgQueue(WorkQueue *queList,char **msg,int *msgSize);
extern void destoryQueue(WorkQueue *queList);

//使用后台线程，初始化的函数(不需要执行queue_init)
extern WorkQueue *InitCondWorkPthread(void handleMsg(const char *msg,int msgSize));

extern void CleanCondWorkPthread(WorkQueue *queList,void cleanMsg(const char *msg,int msgSize));

#endif

