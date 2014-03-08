#GTThreads + Dining Philosophers Problem

Implementation of a user-level thread package - with an API similar to [pthreads](http://man7.org/linux/man-pages/man7/pthreads.7.html). The library is then used for implementing a solution to the classic [Dining Philosophers problem](http://en.wikipedia.org/wiki/Dining_philosophers_problem).


###GTThreads API
  * void gtthread\_init(long period);
  * int  gtthread\_create(gtthread\_t \*thread,void \*(\*start_routine)(void \*),void \*arg);
  * int  gtthread\_join(gtthread\_t thread, void \*\*status);
  * void gtthread\_exit(void \*retval);
  * int  gtthread\_yield(void);
  * int  gtthread\_equal(gtthread\_t t1, gtthread\_t t2);
  * int  gtthread\_cancel(gtthread\_t thread);
  * gtthread\_t gtthread\_self(void);

###Mutex API
  * int  gtthread\_mutex\_init(gtthread\_mutex\_t \*mutex);
  * int  gtthread\_mutex\_lock(gtthread\_mutex\_t \*mutex);
  * int  gtthread\_mutex\_unlock(gtthread\_mutex\_t \*mutex);

###**Development Environment:** 
Ubuntu 12.04 LTS (running on 64-bit x86 hardware).

###**Implementation of preemptive scheduler:**
Preemptive scheduler was constructed using a doubly linked list; wherein the data structure, an extra pointer was maintained to point to the current thread in execution (a.k.a. 'curr'). The scheduler switches from one thread to another at the time of expiration of period (as specified by the user in gtthread_init(period). Also, during thread switches and linked-list updates, it is mandated that all the signals are blocked (and aptly unblocked) in order to avoid inconsistencies. The scheduler's timer is a virtual timer and is set in motion only during thread's execution - at expiration, a SIGVTALRM is raised, acting as a proponent for context switch.

###**Compilation Procedure**:
1. Extract the tarball and run make - if you want to get gtthread.a.
2. Once gtthread.a is produced, run the following command from your Terminal:
  
  gcc -g -Wall -pedantic -I{...} -o diningPhilo diningPhilo.c gtthread.a

###**Preventing deadlocks in Dining Philosophers solution**:
I've used the resource hierarchy solution in order to stay clear of deadlocks. In this solution, all chopsticks are numbered (in this case, from 1-5). Whenever a philosopher feels hungry, he / she is made to pick a chopstick with lower numbering first, for. e.g. Philosopher #1 will acquire Chopstick #1 before he / she tries to acquire Chopstick #5. Similarly, Philosopher #2 will acquire Chopstick #1 before trying to acquire Chopstick #2, and so on. There is no ordering for relinquishing chopsticks; if the order is maintained while picking the chopsticks, the solution doesn't deadlock.

###**Miscellaneous Comments**:
This project was difficult, due to the fact that it had been a while since (undergraduate-level) Linux system programming. I really had to go digging while trying to understand fundamentals of signal handling and context switching :) It was a great learning experience, for the parts that I was able to implement but was endlessly frustrating for the parts that I still can't figure out. I largely ran into trouble with gtthread_join(), and despite many serious efforts, I was not able to implement this portion properly. 



