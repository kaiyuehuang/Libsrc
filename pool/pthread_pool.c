#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <pthread.h> 
#include <assert.h> 
#include <signal.h>

#include "pthread_pool.h"


thread_pool *pool ;

void *thread_routine(void *); 


void *myprocess(void *arg) 
{ 
	printf("I am %u, arg: %d\n", (int)pthread_self(), (int)arg);
	sleep(1);

	return NULL; 
} 


/*****创建线程池**********/

void pool_init(int thread_num) 
{ 
	pool = (thread_pool *)malloc (sizeof(thread_pool)); 

	pthread_mutex_init (&(pool->queue_lock), NULL); 
	pthread_cond_init (&(pool->queue_ready), NULL); 

	pool->queue_head = NULL; 

	pool->thread_num = thread_num; 
	pool->cur_queue_size = 0; 

	pool->shutdown = false; 

	pool->tids = (pthread_t *)malloc(thread_num * sizeof(pthread_t));

	/*
	** create max_thread_num threads
	*/
	int i;
	for(i=0; i<thread_num; i++)
	{
		pthread_create(&(pool->tids[i]), NULL,
			thread_routine ,NULL); 
	
	}
} 



// add task into the pool!
int pool_add_task(void *(*process)(void *), void *arg) 
{ 
	/*
	** instruct a new task structure
	** 
	** fill the field by parameters pass by and
	** set the next to NULL, then add this task
	** node to the thread-pool
	*/
	task *newtask = (task *)malloc(sizeof(task)); 
	newtask->process = process;
	newtask->arg = arg;
	newtask->next = NULL;

	/*
	** the queue is a critical source, thus whenever
	** operates the queue, it should be protected by
	** a mutex or a semaphore.
	*/

	// ====================================== //
	pthread_mutex_lock(&(pool->queue_lock)); 
	// ====================================== //

	/*
	** find the last worker is the pool and then add this
	** one to its tail
	**
	** NOTE: since the worker list which pointed by queue_head
	** has no head-node(which means queue_head could be NULL
	** at first), we should deal with the empty queue carefully.
	*/
	task *last_one = pool->queue_head;
	if(last_one == NULL)
	{
		pool->queue_head = newtask;
	}
	else
	{
		while (last_one->next != NULL)
		{
			last_one = last_one->next;
		}
		last_one->next = newtask; 
	}

	pool->cur_queue_size++; // waiting tasks increase

	// ====================================== //
	pthread_mutex_unlock (&(pool->queue_lock)); 
	// ====================================== //

	// wake up waiting task
	pthread_cond_signal(&(pool->queue_ready));
	

	return 0; 
} 

void *thread_routine(void *arg)
{ 
	while(1)
	{ 
		
		// ====================================== //
		pthread_mutex_lock (&(pool->queue_lock)); 
		// ====================================== //

		/*
		** routine will waiting for a task to run, and the
		** condition is cur_queue_size == 0 and the pool
		** has NOT been shutdowned.
		*/
	    while(pool->cur_queue_size == 0 && !pool->shutdown)
	    {
		    pthread_cond_wait(&(pool->queue_ready), &(pool->queue_lock));
		    
		}

		/*
		** the pool has been shutdowned.
		** unlock before any break, contiune or return
		*/
		if (pool->shutdown)
		{
		    pthread_mutex_unlock (&(pool->queue_lock)); 
		    pthread_exit (NULL); 
		} 

		/*
		** consume the first work in the work link-list
		*/
		pool->cur_queue_size--; 
		task *p = pool->queue_head; 
		pool->queue_head = p->next; 

		// ====================================== //
		pthread_mutex_unlock(&(pool->queue_lock)); 
		// ====================================== //


		/*
		** Okay, everything is ready, now excutes the process
		** from the worker, with its argument.
		*/
		(*(p->process))(p->arg); 

		/*
		** when the work is done, free the source and continue
		** to excute another work.
		*/
		free(p); 
		p = NULL; 
	} 

	pthread_exit(NULL); 
} 


/*
** tasks waiting in the queue will be discarded
** but will wait for the tasks which are still
** running in the pool
*/
int pool_destroy(void) 
{ 
	// make sure it wont be destroy twice
	if(pool->shutdown) 
	   return -1;

	pool->shutdown = true;  // set the flag

	// wake up all of the tasks
	pthread_cond_broadcast(&(pool->queue_ready)); 


	// wait for all of the task exit
	int i; 
	for(i=0; i < pool->thread_num; i++) 
		pthread_join(pool->tids[i], NULL); 

	free(pool->tids); 

	// destroy the queue
	task *p = NULL; 
	while (pool->queue_head != NULL) 
	{ 
		p = pool->queue_head; 
		pool->queue_head = pool->queue_head->next; 
		free(p); 
	} 

	// destroy the mutex and cond
	pthread_mutex_destroy(&(pool->queue_lock)); 
	pthread_cond_destroy(&(pool->queue_ready)); 
	
	free(pool); 
	pool=NULL; 
	return 0; 
} 

int pthread_create_attr(void *(*start_routine) (void *),void *arg)
{
	pthread_t pthread_pid;
	pthread_attr_t attr_pid;	
	pthread_attr_init(&attr_pid);
	pthread_attr_setdetachstate(&attr_pid, PTHREAD_CREATE_DETACHED);
	return pthread_create(&pthread_pid,&attr_pid,start_routine,arg);
}

void set_pthread_sigblock(void)
{
	sigset_t mask, oldmask;
	sigemptyset(&mask);
  	sigaddset(&mask, SIGPIPE);
  	if (pthread_sigmask(SIG_BLOCK, &mask, &oldmask) != 0)
    		printf("SIG_BLOCK error.\n");

}

