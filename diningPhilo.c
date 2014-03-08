#include "gtthread.h"

gtthread_t philosophers[5];
gtthread_mutex_t chopsticks[5];

/*Philosophers acquire chopsticks in a manner where the 'lower numbered' chopstick is picked before a 'higher numbered' chopstick.
for e.g. Philosopher #1 will acquire Chopstick #1 before he / she tries to acquire Chopstick #5. Similarly, Philosopher #2 will acquire Chopstick #1 before trying to acquire Chopstick #2, and so on.
*/
void acquireAndEat(gtthread_mutex_t lChopstick,gtthread_mutex_t rChopstick, int philosopher)
{
	printf("Philosopher #%d is hungry!\n",philosopher); fflush(stdout);
	if (lChopstick<rChopstick)
	{
		printf("Philosopher #%d tries to acquire chopstick %d\n",philosopher,lChopstick+1); fflush(stdout);
		gtthread_mutex_lock(&lChopstick);
		printf("Philosopher #%d tries to acquire chopstick %d\n",philosopher,rChopstick+1); fflush(stdout);
		gtthread_mutex_lock(&rChopstick);
		printf("Philosopher #%d eating with chopstick %d and %d\n",philosopher,lChopstick+1,rChopstick+1); fflush(stdout);
	}
	else
	{
		printf("Philosopher #%d tries to acquire chopstick %d\n",philosopher,rChopstick+1); fflush(stdout);
		gtthread_mutex_lock(&rChopstick);
		printf("Philosopher #%d tries to acquire chopstick %d\n",philosopher,lChopstick+1); fflush(stdout);
		gtthread_mutex_lock(&lChopstick);
		printf("Philosopher #%d eating with chopstick %d and %d\n",philosopher,lChopstick+1,rChopstick+1); fflush(stdout);
	}
	gtthread_yield();
}

void releaseAndThink(gtthread_mutex_t lChopstick,gtthread_mutex_t rChopstick, int philosopher)
{
	printf("Philosopher #%d releasing chopstick %d\n",philosopher,rChopstick+1); fflush(stdout);
	gtthread_mutex_unlock(&rChopstick);
	printf("Philosopher #%d releasing chopstick %d\n",philosopher,lChopstick+1); fflush(stdout);
	gtthread_mutex_unlock(&lChopstick);
	printf("Philosopher #%d thinking\n",philosopher); fflush(stdout);
	gtthread_yield();
}

void* diningPhilosphers(void *arg)
{
	gtthread_mutex_t lChopstick,rChopstick;
	/* 
	Table arrangement :
	Philosopher 1: Would have Chopstick #5 and Chopstick #1 around his / her plate
	Philosopher 2: Would have Chopstick #1 and Chopstick #2 around his / her plate
	Philosopher 3: Would have Chopstick #2 and Chopstick #3 around his / her plate
	Philosopher 4: Would have Chopstick #3 and Chopstick #4 around his / her plate
	Philosopher 5: Would have Chopstick #4 and Chopstick #5 around his / her plate
	*/
	switch((int)arg)
	{
		case 1: lChopstick=chopsticks[4]; rChopstick=chopsticks[0]; break;
		case 2: lChopstick=chopsticks[0]; rChopstick=chopsticks[1]; break;
		case 3: lChopstick=chopsticks[1]; rChopstick=chopsticks[2]; break;
		case 4: lChopstick=chopsticks[2]; rChopstick=chopsticks[3]; break;
		case 5: lChopstick=chopsticks[3]; rChopstick=chopsticks[4]; break;
	}
	while(1)
	{
		acquireAndEat(lChopstick,rChopstick,(int)arg);

		releaseAndThink(lChopstick,rChopstick,(int)arg);
		
	}
}

int main(){

	gtthread_init(99999);

	int i=0;
	while (i<5)
	{
		gtthread_mutex_init(chopsticks+i);	
		gtthread_create(philosophers+i, diningPhilosphers, (void*)(i+1));
		i++;
	}
	while(1);
}
