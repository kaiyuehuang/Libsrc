#ifndef _PTHREAD_POOL_H
#define _PTHREAD_POOL_H

#include <stdbool.h> 
#include <pthread.h>



/************************************************
** A pool contains some following structure nodes 
** which are linked together as a link-list, and
** every node includes a thread handler for the
** task, and its argument if it has any.
*************************************************/
typedef struct task
{ 
	void *(*process) (void *); 
	void *arg;	// arguments for a task 

	struct task *next; 

}task; 

/*
** the thread pool
*/
typedef struct 
{ 
	pthread_mutex_t queue_lock; 
	pthread_cond_t  queue_ready; 

	// all waiting tasks
	task *queue_head; 

	/*
	** this field indicates the thread-pool's
	** state, if the pool has been shut down
	** the field will be set to true, and it is
	** false by default.
	*/
	bool shutdown; 

	// containning all threads's tid
	pthread_t *tids; 

	// max active taks
	int thread_num; 

	// current waitting tasks
	int cur_queue_size; 

}thread_pool; 



extern thread_pool *pool ;
extern int pool_add_task(void *(*process)(void *), void *arg) ;

extern void *myprocess(void *arg) ;

extern void pool_init(int thread_num);

extern int pool_destroy(void) ;

extern int pthread_create_attr(void *(*start_routine) (void *),void *arg);

extern void set_pthread_sigblock(void);

#endif /*end for pthread_pool.h*/
