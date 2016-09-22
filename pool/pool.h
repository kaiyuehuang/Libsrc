#ifndef _POOL_H_
#define _POOL_H_

#define MEM_POOL_SIZE 1500
typedef unsigned short uint16;

#define QUE_POOL sizeof(queuelist)

typedef struct queuenode
{
	char *data;
	int size;
	struct queuenode *next;
}queuelist;

typedef struct queue
{	
	queuelist Head;	
	queuelist *Tail;
	int block_size;
	uint16 init_size;
	uint16 grow_size;//Ö®ºó´´½¨BlockÊ±µÄchunkÊý
	uint16 free;
	uint16 full;
	pthread_mutex_t H_Mutex;
	pthread_mutex_t T_Mutex;
}memlink;
extern memlink recvhead;
extern void init_poollist(memlink *phead,int block_size,uint16 init_size,uint16 grow_size);

extern void *get_mem(memlink *phead);

extern void free_mempool(memlink *phead,queuelist *block);

extern void destory_mempool(memlink *phead);

/******************************************************************************************
===========================================================================================

******************************************************************************************/
extern void *myprocess(void *arg) ;

extern int pool_add_task(void *(*process)(void *), void *arg) ;

extern void pool_init(int thread_num);

extern int pool_destroy(void) ;

extern int pthread_create_attr(void *(*start_routine) (void *),void *arg);
extern void set_pthread_sigblock(void);

#endif
