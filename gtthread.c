#include "gtthread.h"

struct threadDS threadPool[MAXTHREADCOUNT];
gtthread_mutex_t midInUse[MAXMUTEXCOUNT];
gtthread_t tidInUse[MAXTHREADCOUNT];

struct doublyLinkedList refBlock;
struct doublyLinkedList *ref;

long quantum=-1;

#if 0
----------------------------------------------------------------------------------------------------------------
definitions : helpers
----------------------------------------------------------------------------------------------------------------
#endif

int getUnusedTID()
{
	int i=0; 
	while(i<MAXTHREADCOUNT)
	{
		if(!tidInUse[i])
			{ 
				tidInUse[i]=1; 
				return i;
			}
		i++;
	}
	return 0;
}

int getUnusedMID()
{
	int i=0;
	while(i<MAXMUTEXCOUNT)
	{
		if(!midInUse[i])
			{ 
				midInUse[i]=1;
				return i; /*return the index of first unused mutex in 'midInUse'.*/
			}
		i++;
	}
	return 0;
}

void executeRoutine(void *(*start_routine)(void *),void *arg)
{
	void *retval=start_routine(arg);
	gtthread_exit(retval);
}


#if 0
----------------------------------------------------------------------------------------------------------------
definitions : gtthread specific
----------------------------------------------------------------------------------------------------------------
#endif

void gtthread_init(long period)
{
	if(period>0 && quantum<0)
	{
		quantum=period;

		memset(midInUse,0,sizeof(gtthread_mutex_t)*MAXMUTEXCOUNT);
		memset(tidInUse,0,sizeof(gtthread_t)*MAXTHREADCOUNT);
		memset(threadPool,0,sizeof(struct threadDS)*MAXTHREADCOUNT);

		int temp=getUnusedTID();
		
		struct threadDS *newthread=(threadPool+temp); /*Over here, newthread == main()*/
		newthread->tid=temp;
		newthread->complete=0;
		newthread->noOfJoinedThread=0;
		newthread->pid=newthread->tid;
		
		ref=&refBlock;

		ref->curr=newthread;
		ref->activeThreadCount=0;
		ref->exitRequest=0;

		addThread(newthread);
		
		initializeTimer(period);
	}
	else{
		perror("Invalid period.\n");
		exit(1);
	}
}


int gtthread_create(gtthread_t *x, void *(*start_routine)(void *), void *arg)
{
	int temp=getUnusedTID();

	struct threadDS *newthread=(threadPool+temp); /*make newThread point to the appropriate slot in threadPool*/
	newthread->tid=temp;
	newthread->complete=0;
	newthread->pid=ref->curr->tid;
	newthread->noOfJoinedThread=0;
	
	if(x)
	 *x=newthread->tid;

	getcontext(&newthread->tctxt);
	newthread->tctxt.uc_link=0;
	newthread->tctxt.uc_stack.ss_sp=malloc(CONTEXTSTACK);
	newthread->tctxt.uc_stack.ss_size=CONTEXTSTACK;
	newthread->tctxt.uc_stack.ss_flags=0;
	makecontext(&newthread->tctxt,executeRoutine,2,start_routine,arg);

	if(!newthread->tctxt.uc_stack.ss_sp)
	{
		printf("malloc() woes!\n");
		exit(1);
	}

	addThread(newthread);
	return 0;
}

int gtthread_equal(gtthread_t t1,gtthread_t t2)
{
	return (t1==t2)?1:0;
}

gtthread_t gtthread_self()
{
	return ref->curr->tid;
}

void gtthread_yield(void)
{
	raise(SIGVTALRM);
}

int  gtthread_join(gtthread_t tid, void **status)
{
	struct threadDS *toBeJoined=threadPool+tid;
	threadPool[toBeJoined->pid].noOfJoinedThread+=1; /**/
	
	while(toBeJoined->complete!=1);
	
	if(*status) *status=toBeJoined->ret;
	
	return 0;
}

void gtthread_exit(void *retval)
{
	ref->curr->ret=retval;
	ref->curr->complete=1;

	while(ref->curr->noOfJoinedThread>0); /*while there are more sub-threads executing, wait!*/
	
	if(ref->curr->tid==ref->head->tid)
	{
		ref->exitRequest=1;
		/*if current == main*/
		exit(0);
	} 
	removeThread(ref->curr->tid);
}

int  gtthread_cancel(gtthread_t t)
{
	struct threadDS *toBeCanceled=threadPool+t;
	toBeCanceled->ret=(void*)"CANCELED";
	toBeCanceled->complete=1;
	
	while(toBeCanceled->noOfJoinedThread>0);

	removeThread(t); /*when all the sub-threads are canceled / removed, remove the current thread*/
	return 0;
}

#if 0
----------------------------------------------------------------------------------------------------------------
definitions : mutex specific
----------------------------------------------------------------------------------------------------------------
#endif

int  gtthread_mutex_init(gtthread_mutex_t *index)
{
	*index=getUnusedMID();
	return 0;
}

int  gtthread_mutex_lock(gtthread_mutex_t *index)
{
	while(*(midInUse+(*index))!=1); /*check till tidInUse[*index] is not set to 1 => 1 implies that the mutex on position 'index' has been initialized, but unused currently*/
	*(midInUse+(*index))=ref->curr->tid+1;
	return 0;
}

int  gtthread_mutex_unlock(gtthread_mutex_t *index)
{
	if(*(midInUse+(*index))==ref->curr->tid+1)
	{
		*(midInUse+(*index))=1; 
		return 0;
	}
	return 1;
}

#if 0
----------------------------------------------------------------------------------------------------------------
definitions : scheduler specific
----------------------------------------------------------------------------------------------------------------
#endif

void initializeTimer(long period)
{
	quantum=period;
	sigemptyset(&signalSet);
	sigaddset(&signalSet,SIGVTALRM);
	timer.it_value.tv_sec=0;
	timer.it_value.tv_usec=period;
	timer.it_interval.tv_sec=0;
	timer.it_interval.tv_usec=period;
	signal(SIGVTALRM,switchContext);
	setitimer(ITIMER_VIRTUAL,&timer,NULL); 

}

void switchContext(int sig){
	
	sigprocmask(SIG_BLOCK, &signalSet, NULL); /*change the signal mask of calling thread - block it */

	/*check if the current process has not exited (exitRequest==0) AND that there are threads which can be used for switching the current context to theirs.*/
	if(ref->exitRequest==0 && (ref->activeThreadCount>1 || (ref->activeThreadCount==1 && ref->curr->tid==ref->curr->next->tid)))
	{
		ref->curr=ref->curr->next; /*move a position ahead*/

		initializeTimer(quantum); /*Reset timer - Give each thread a time-slice of 'qunatum'*/
		
		sigprocmask(SIG_UNBLOCK, &signalSet, NULL);	

		swapcontext(&ref->curr->prev->tctxt,&ref->curr->tctxt); /*swap context with thread existing before it in the threadPool.*/
	}
	else
	{
		ref->curr=ref->curr->next;
		ref->exitRequest=0;
		
		initializeTimer(quantum); /*Reset timer - Give each thread a time-slice of 'qunatum'*/

		sigprocmask(SIG_UNBLOCK, &signalSet, NULL);	 /*change the signal mask of calling thread - unblock it. --> Timer Elapsed*/

		swapcontext(&inactiveContext,&ref->curr->tctxt);
	}
}

void addThread(struct threadDS *newThread)
{
	sigprocmask(SIG_BLOCK, &signalSet, NULL); /*change the signal mask of calling thread - block it*/

	if(ref->activeThreadCount==0)
	{
		newThread->next=newThread;
		newThread->prev=newThread;

		ref->head=newThread;
		ref->tail=newThread;
	}
	else
	{
		newThread->next=ref->head;
		newThread->prev=ref->tail;

		ref->tail->next=newThread;
		ref->head->prev=newThread;

		ref->tail=newThread;
	}
	(threadPool+(newThread->pid))->noOfJoinedThread+=1;
	
	ref->activeThreadCount++; 
	
	sigprocmask(SIG_UNBLOCK, &signalSet, NULL);	/*change the signal mask of calling thread - unblock it. --> Insertion Elapsed*/
}

void removeThread(gtthread_t id)
{
	sigprocmask(SIG_BLOCK, &signalSet, NULL); /*change the signal mask of calling thread - block it*/

	struct threadDS *temp=ref->head;

	do
	{
		if(temp->tid==id)
			break;
		temp=temp->next;
	}
	while(temp!=ref->head && temp->tid!=id);

	while (temp->noOfJoinedThread>0); /*Wait while there are threads joined to thread with tid "id"*/

	if (ref->activeThreadCount==1)
	{
		ref->head=ref->tail=ref->curr=NULL;
	}
	else
	{
		if (temp==ref->head)
		{
			ref->head=temp->next;
			ref->head->prev=ref->tail;
			ref->tail->next=ref->head;
		}
		if (temp==ref->tail)
		{
			ref->tail=temp->prev;
			ref->tail->next=ref->head;
			ref->head->prev=ref->tail;
		}
		else
		{
			temp->prev->next=temp->next;
			temp->next->prev=temp->prev;
		}
		(threadPool+(temp->pid))->noOfJoinedThread-=1;
	}
	ref->activeThreadCount-=1;
	
	sigprocmask(SIG_UNBLOCK, &signalSet, NULL);	/*change the signal mask of calling thread - unblock it. --> Removal complete*/

	if (ref->curr==temp)
				raise(SIGVTALRM);
}