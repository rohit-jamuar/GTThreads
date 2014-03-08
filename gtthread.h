#ifndef __GTTHREAD_HEADER_H
#define __GTTHREAD_HEADER_H

#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ucontext.h>

#define MAXMUTEXCOUNT 100000
#define MAXTHREADCOUNT 100000
#define CONTEXTSTACK 8192

typedef int gtthread_mutex_t;
typedef unsigned long gtthread_t;

struct itimerval timer;
ucontext_t inactiveContext;
sigset_t signalSet;

struct threadDS
{
	gtthread_t tid; /*thread id*/
	ucontext_t tctxt; /*thread's context*/
	void *ret; /*return value (when returning post successful execution / cancellation)*/
	int complete; /*is 0, when the thread is active; 1 otherwise.*/
	int noOfJoinedThread; /*a thread can have multiple threads stemming from it*/
	gtthread_t pid; /*parent's id*/
  	struct threadDS *next; /*for linking with other threads which have been created from thread with id : pid*/
	struct threadDS *prev; /*for linking with other threads which have been created from thread with id : pid*/
};

struct doublyLinkedList /*for keeping track of thread(s)/mutex(es)*/
{
	struct threadDS *head;
	struct threadDS *tail;
	struct threadDS *curr; /*points to the thread executing currently*/
	unsigned long activeThreadCount;
	int exitRequest; /*set to 1 when gtthread_exit() is called*/
};

/*gtthread routines*/
void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);

/*mutex routines*/
int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);

/*scheduler routines*/
void switchContext(int sig);
void initializeTimer(long period);
void addThread(struct threadDS *newThread);
void removeThread(gtthread_t id);

/*additional helpers*/
int getUnusedMID();
int getUnusedTID();
void executeRoutine(void *(*start_routine)(void *),void *arg);

#endif