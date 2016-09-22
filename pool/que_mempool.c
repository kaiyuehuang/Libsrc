#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>

#include "que_mempool.h"

//#define MEM_DEBUGH 
memlink recvhead;

/********加入队列************/
static int entry_queue(memlink *phead,queuelist *cur)
{	
	if(cur==NULL)		
		return 0;
	cur->next =NULL;
	/*********给队尾上锁*******/
	pthread_mutex_lock(&phead->T_Mutex);
	phead->Tail->next= cur;
	phead->Tail = cur;
	phead->free +=1;
	pthread_mutex_unlock(&phead->T_Mutex);	
#ifdef  MEM_DEBUGH	
	printf("queque sucess\n");
#endif
	return 1;
}
/********创建内存池**************/
static void mem_create(memlink *phead,int chunksize,uint16 num)
{
    //memblock长度 
    int length=sizeof(struct queuenode);

#ifdef MEM_DEBUGH
	printf("length = %d\n",length);
#endif
	
	int i;
	for(i=0;i<num;i++)
	{
		struct queuenode* block=malloc(length);
		block->size = chunksize;
		block->data = malloc(chunksize);
    		memset(block->data,0,block->size);
		if (block==NULL)
		{
    			printf("%s,malloc error.",__func__);
    			continue;
    		}
    		entry_queue(phead,block);
    		phead->full+=1;
	}
}


/*************************************
chunksize :每一块内存大小 
growsize :之后创建block时的chunk的个数
**************************************/

void init_poollist(memlink *phead,int block_size,uint16 init_size,uint16 grow_size)
{	
	if(!init_size || !grow_size)
		return ;
		
	phead->Tail = &phead->Head;
	phead->init_size = init_size; //初始值创建内存池大小
	phead->grow_size = grow_size; //内存不足是新增内存池大小
	phead->block_size = block_size; //内存不足是新增内存池大小
	phead->free =0;
	phead->full=0;
	pthread_mutex_init(&(phead->H_Mutex),NULL); 
	pthread_mutex_init(&(phead->T_Mutex),NULL); 

	mem_create(phead,block_size,init_size);
	
	#ifdef MEM_DEBUGH
		printf("mem pool size = %d ,mem pool num = %d\n",block_size,init_size);
	#endif
}
int is_empty(memlink *phead)
{	
	if(phead->Head.next ==NULL) 
		return 1;	
	else 
		return 0;
}
/*********************************************
获取内存
*********************************************/
void *get_mem(memlink *phead)
{
#ifdef MEM_DEBUGH
	printf("get_mem phead->free = %d\n",phead->free);
#endif
	pthread_mutex_lock(&phead->H_Mutex);
	/*******内存池不足，自动申请内存************/
/*	if(phead->free ==0)
	{
		return NULL;
	}
*/
	struct queuenode *mem;
	mem = phead->Head.next;
	phead->Head.next = phead->Head.next->next;
	
	if(phead->Head.next == NULL)  
	{
	    phead->Tail = &phead->Head;
		pthread_mutex_unlock(&phead->H_Mutex);
		mem_create(phead,phead->block_size,phead->grow_size);
		pthread_mutex_lock(&phead->H_Mutex);
		printf("create mem after size =%d\n",phead->grow_size);
    	//return NULL;
	}
	
	phead->free -=1;
	pthread_mutex_unlock(&phead->H_Mutex);
	return mem;
}
/**********************************************
回收内存
**********************************************/
void free_mempool(memlink *phead,queuelist *block)
{
	memset(block->data,0,block->size);
#ifdef  MEM_DEBUGH	
	printf("free_mempool 0x%p\n",block->data);
#endif
	entry_queue(phead,block);
}
/************销毁内存池*************************/
void destory_mempool(memlink *phead)
{
	int i=0;
	struct queuenode *mem;
	while(phead->Head.next!=NULL)
	{	
		mem = phead->Head.next;
		phead->Head.next = phead->Head.next->next;
		free(mem->data);
		free(mem);
		++i;
		#ifdef MEM_DEBUGH
			printf("num pool = %d\n",i);
		#endif
	}
	if(i==phead->full)
	{
		printf("destory_mempool:  finish number=%d \n",i);
	}else{
		printf("destory_mempool:  less memory release number=%d full number=%d\n",i,phead->full);
	}
}
#if 0
void *get1(void *arg)
{
	int i=0;
	while(1)
	{
		if(i++>100)
			break;
		queuelist *q = (queuelist *)get_mem(&recvhead);
		if(q!=NULL)
		{
			memcpy(q->data,"adfsdfddsdf",10);
			free_mempool(&recvhead,(queuelist *)q);
			printf("thread 1\n");
			usleep(10);
		}
	}
	return NULL;
}

void *get2(void *arg)
{
	int i=0;
	while(1)
	{
		if(i++>100)
			break;
		queuelist *f = (queuelist *)get_mem(&recvhead);
		if(f!=NULL)
		{
			memcpy(f->data,"adfsdfddsdf",10);
			free_mempool(&recvhead,(queuelist *)f);
			printf("thread 2\n");
			usleep(10);
		}
	}
	return NULL;
}
void *get3(void *arg)
{
	int i=0;
	while(1)
	{
		if(i++>100)
			break;
		queuelist *m = (queuelist *)get_mem(&recvhead);
		if(m!=NULL)
		{
			memcpy(m->data,"adfsdfddsdf",10);
			usleep(10);
			free_mempool(&recvhead,(queuelist *)m);
			printf("thread 3\n");
			
		}
	}
	return NULL;
}
void *get4(void *arg)
{
	int i=0;
	while(1)
	{
		if(i++>100)
			break;
		queuelist *n =(queuelist *) get_mem(&recvhead);
		if(n!=NULL)
		{
			memcpy(n->data,"adfsdfddsdf",10);
			usleep(10);
			free_mempool(&recvhead,(queuelist *)n);
			printf("thread 4 \n");
		}
	}
	return NULL;
}

int main(void)
{
	pthread_t tid1,tid2,tid3,tid4;
	
	init_poollist(&recvhead,100,2,2);
	queuelist *p = (queuelist *)get_mem(&recvhead);
	
	queuelist *q = (queuelist *)get_mem(&recvhead);
	
	queuelist *h = (queuelist *)get_mem(&recvhead);
	
	memcpy(p->data,"test pool memory ",20);	
	printf("p->mem = %s\n",p->data);
	printf("p->size = %d\n",p->size);
    printf("q->mem = %s\n",q->data);
	printf("q->size = %d\n",q->size);
    printf("h->mem = %s\n",h->data);
	printf("h->size = %d\n",h->size);
	free_mempool(&recvhead,(queuelist *)p);
	free_mempool(&recvhead,(queuelist *)q);
	//free_mempool(&recvhead,(queuelist *)h);
#if 0
	pthread_create(&tid1,NULL,get1,NULL);
	pthread_create(&tid2,NULL,get2,NULL);
	pthread_create(&tid3,NULL,get3,NULL);
	pthread_create(&tid4,NULL,get4,NULL);

	sleep(50);
#endif
	destory_mempool(&recvhead);	
	return 0;
}
#endif
