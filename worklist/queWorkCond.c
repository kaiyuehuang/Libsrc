#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "queWorkCond.h"

#define DBG_QUEMSG
#ifdef DBG_QUEMSG
#define DEBUG_QUEMSG(fmt, args...) printf("queMsg: " fmt, ## args)
#else
#define DEBUG_QUEMSG(fmt, args...) { }
#endif
int getWorkMsgNum(WorkQueue *queList)
{
    return queList->cacheWorkSize;
}

WorkQueue *initQueue(void)
{
	WorkQueue *q = (WorkQueue *)calloc(1,sizeof(WorkQueue));;
    if (q == NULL)
    {
        perror("Init_WorkPthread failed");
        return NULL;
    }
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    q->worktail = &q->workhead;
    q->workhead.next = NULL;
	return q;
}
int putMsgQueue(WorkQueue *queList, const char *msg, int msgSize)
{
    WorkMsg *cur = (WorkMsg *)malloc(WORKMSG_SIZE);
    if (cur == NULL)
    {
        perror("workMsg  malloc  failed");
        return -1;
    }
    memset(cur, 0, WORKMSG_SIZE);
    cur->msg = msg;
    cur->msgSize = msgSize;
    pthread_mutex_lock(&queList->mutex);

    queList->worktail->next = cur;
    queList->worktail = cur;

    ++queList->cacheWorkSize;
    pthread_cond_signal(&queList->cond);
    pthread_mutex_unlock(&queList->mutex);

    //DEBUG_QUEMSG("putMsgQueue success \n");
    return 0;
}
int getMsgQueue(WorkQueue *queList,char **msg,int *msgSize)
{
	WorkMsg *workmsg;
	pthread_mutex_lock(&queList->mutex);
	while(queList->cacheWorkSize == 0)
		pthread_cond_wait(&queList->cond, &queList->mutex);

	workmsg = queList->workhead.next;
	queList->workhead.next = queList->workhead.next->next;
	
	if (queList->workhead.next == NULL) queList->worktail = &queList->workhead;
	
	pthread_mutex_unlock(&queList->mutex);
	--queList->cacheWorkSize;
	*msg = workmsg->msg;
	*msgSize=workmsg->msgSize;
	free(workmsg);
	return 0;
}
void destoryQueue(WorkQueue *queList)
{
	if(queList)
	{
		pthread_cond_destroy(&queList->cond);
    	pthread_mutex_destroy(&queList->mutex);
    	free(queList);
		queList=NULL;
	}
}
/********************************************
处理udp队列里的信息客户端
********************************************/
static void* handle_queList(void *arg)
{
    WorkQueue *queList = (WorkQueue *)arg;
	char *msg =NULL;
	int msgSize=0;
    while (1)
    {
    	getMsgQueue(queList,&msg,&msgSize);
		if (queList->pthread_live == 0)
        {
            goto handle_exit;
        }
        DEBUG_QUEMSG("handle message \n");
        queList->callFuntion((const char *)msg, msgSize);
    }
handle_exit:
    printf("handle msg exit \n");
    queList->pthread_live = 2;
    return NULL;
}

static int pthread_create_attr(void* (*start_routine)(void *), void *arg)
{
    pthread_t pthread_pid;
    pthread_attr_t attr_pid;
    pthread_attr_init(&attr_pid);
    pthread_attr_setdetachstate(&attr_pid, PTHREAD_CREATE_DETACHED);
    return pthread_create(&pthread_pid, &attr_pid, start_routine, arg);
}

WorkQueue* InitCondWorkPthread(void handleMsg(const char *msg, int msgSize))
{
	WorkQueue *queList = initQueue();
	if(queList==NULL)
		return NULL;
    queList->pthread_live = 1;

    queList->callFuntion = handleMsg;
    queList->cacheWorkSize = 0;

    if (pthread_create_attr(handle_queList, queList))
    {
        perror("create handle_udpmsg pthread failed ");
        goto exit;
    }
    usleep(1000);
    return queList;
exit:
    free(queList);
    return NULL;
}

void CleanCondWorkPthread(WorkQueue *queList, void cleanMsg(const char *msg, int msgSize))
{
    WorkMsg *workmsg;
    queList->pthread_live = 0;
	usleep(100);
	char *quit =(char *)malloc(5);
	sprintf(quit,"%s","quit");
    putMsgQueue(queList,quit,0);
    while (1)
    {
        if (queList->pthread_live != 2)
        {
            printf("wait que pthread exit\n");
            usleep(1000);
            continue;
        }
		
        workmsg = queList->workhead.next;
        if (workmsg == NULL) break;
        DEBUG_QUEMSG("free cache que message ing\n");
        queList->workhead.next = queList->workhead.next->next;
        cleanMsg(workmsg->msg, workmsg->msgSize);
        free(workmsg);
    }
	destoryQueue(queList);
    DEBUG_QUEMSG("free cache que message finnish\n");
}

#if 0
void handleMsg(const char *msg, int msgSize)
{
    printf("msg = %s\n", msg);
    free(msg);
}
void cleanMsg(const char *msg, int msgSize)
{
    printf("clean msg = %s\n", msg);
    free(msg);
}

void test_CondWorkPthread(void)
{
    WorkQueue *qlist = InitCondWorkPthread(handleMsg);
    int i = 0;
    for (i = 0; i < 100; i++)
    {
        char *msg = malloc(100);
        sprintf(msg, "%s%d", "add ", i);
        putMsgQueue(qlist, (const char *)msg, 0);
        usleep(1);
    }
    CleanCondWorkPthread(qlist, cleanMsg);

    sleep(1);
    printf("exit ok \n");
}

void *testQueList(void *arg)
{
	WorkQueue *queList  =(WorkQueue *)arg;
	char *msg =NULL;
	int msgSize=0;
	while(1)
	{
		getMsgQueue(queList,&msg,&msgSize);
		printf("msg = %s msgSize = %d\n", msg,msgSize);
		free(msg);
	}
}
void test_CondWork(void)
{
    WorkQueue *queList = initQueue();
	int i = 0;
    if (pthread_create_attr(testQueList, queList))
    {
        perror("create handle_udpmsg pthread failed ");
    }
    
    for (i = 0; i < 10000; i++)
    {
        char *msg = malloc(100);
        sprintf(msg, "%s%d", "add ", i);
		//printf("add ok [%d]\n",i);
        putMsgQueue(queList, (const char *)msg, 0);
        usleep(1);
    }
   
    sleep(1);
    printf("exit ok \n");
}

int main(void)
{
	//test_CondWork();
	test_CondWorkPthread();
    return 0;
}
#endif
