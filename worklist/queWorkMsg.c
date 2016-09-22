#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "queWorkMsg.h"
#define WORK_MAXFULL 100
#define WORK_EMPTY 0
#define WORKUDP_SIZE sizeof(Workudp)

//#define DBG_QUEMSG
#ifdef DBG_QUEMSG
#define DEBUG_QUEMSG(fmt, args...) printf("queMsg: " fmt, ## args)
#else
#define DEBUG_QUEMSG(fmt, args...) { }
#endif
int get_WorkMsgNum(queworklist *queList)
{
    return queList->listNum;
}

/***************************************
将接收到udp控制信息，
添加到udp信息控制的工作队列
****************************************/
int Add_WorkMsg(queworklist *queList, const char *msg, int msgSize)
{
    ++queList->listNum; 
    if (queList->listNum >= WORK_MAXFULL)
    {
        return -1;
    }
    Workudp *cur = (Workudp *)malloc(WORKUDP_SIZE);
    if (cur == NULL)
    {
        perror("workudp  malloc  failed");
        return -2;
    }
    memset(cur, 0, WORKUDP_SIZE);
    cur->msg = msg;
    cur->msgSize = msgSize;
    sem_wait(&queList->Full);
    pthread_mutex_lock(&queList->workmutex);

    queList->worktail->next = cur;
    queList->worktail = cur;

    sem_post(&queList->Empty);
    pthread_mutex_unlock(&queList->workmutex);

    DEBUG_QUEMSG("Add_WorkMsg success \n");
    return 0;
}
/********************************************
处理udp队列里的信息客户端
********************************************/
static void* handle_queList(void *arg)
{
    queworklist *queList = (queworklist *)arg;
    Workudp *workmsg;
    //callFuntion pcall =(callFuntion *)arg;
    while (1)
    {
        sem_wait(&queList->Empty);
        pthread_mutex_lock(&queList->workmutex);

        if (queList->pthread_live == 0)
        {
            goto handle_exit;
        }
        workmsg = queList->workhead.next;
        queList->workhead.next = queList->workhead.next->next;

        if (queList->workhead.next == NULL) queList->worktail = &queList->workhead;

        sem_post(&queList->Full);
        pthread_mutex_unlock(&queList->workmutex);
        --queList->listNum;

        DEBUG_QUEMSG("handle message \n");

        queList->callFuntion(workmsg->msg, workmsg->msgSize);

        free(workmsg);
        workmsg = NULL;
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

queworklist* Init_WorkPthread(void handleMsg(const char *msg, int msgSize))
{
    queworklist *queList = (queworklist *)malloc(sizeof(queworklist));
    if (queList == NULL)
    {
        perror("Init_WorkPthread failed");
        return NULL;
    }
    queList->pthread_live = 1;
    queList->listNum = 0;
    queList->worktail = &queList->workhead;
    queList->workhead.next = NULL;

    sem_init(&queList->Empty, 0, WORK_EMPTY);
    sem_init(&queList->Full, 0, WORK_MAXFULL);
    pthread_mutex_init(&(queList->workmutex), NULL);

    queList->callFuntion = handleMsg;

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

void Clean_WorkPthread(queworklist *queList, void cleanMsg(const char *msg, int msgSize))
{
    Workudp *workmsg;
    queList->pthread_live = 0;
    sem_post(&queList->Empty);
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
    sem_destroy(&queList->Empty);
    sem_destroy(&queList->Full);
    pthread_mutex_destroy(&queList->workmutex);
    free(queList);
    DEBUG_QUEMSG("free cache que message finnish\n");
}

#define MAIN_TEST
#ifdef MAIN_TEST
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
int main(void)
{
    queworklist *qlist = Init_WorkPthread(handleMsg);
    int i = 0;
    for (i = 0; i < 100000; i++)
    {
        char *msg = malloc(100);
        sprintf(msg, "%s%d", "add ", i);
        Add_WorkMsg(qlist, (const char *)msg, 0);
        usleep(1);
    }
    Clean_WorkPthread(qlist, cleanMsg);

    sleep(1);
    printf("exit ok \n");
    return 0;
}
#endif
