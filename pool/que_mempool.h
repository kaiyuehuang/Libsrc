#ifndef _QUE_MEMPOOL_H_
#define _QUE_MEMPOOL_H_

typedef unsigned short uint16;

#define QUE_POOL sizeof(queuelist)

typedef struct queuenode{
	void *data;
	int size;
	struct queuenode *next;
}queuelist;

typedef struct queue{	
	queuelist Head;	
	queuelist *Tail;
	int block_size;
	uint16 init_size;
	uint16 grow_size;//之后创建Block时的chunk数
	uint16 free;
	uint16 full;
	pthread_mutex_t H_Mutex;
	pthread_mutex_t T_Mutex;
}memlink;

extern void init_poollist(memlink *phead,int block_size,uint16 init_size,uint16 grow_size);

extern void *get_mem(memlink *phead);

extern void free_mempool(memlink *phead,queuelist *block);

extern void destory_mempool(memlink *phead);

#endif

