#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "mythread.h"
#include "interrupt.h"

void scheduler();
void timer_interrupt(int sig);

/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N]; 
/* Current running thread */
static int current = 0;
/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init=0;

/* Set the timer */
void reset_timer(long usec) {
  struct itimerval quantum;

  /* Intialize an interval corresponding to round-robin quantum*/
  quantum.it_interval.tv_sec = usec / 1000000;
  quantum.it_interval.tv_usec = usec % 1000000;
  /* Initialize the remaining time from current quantum */
  quantum.it_value.tv_sec = usec / 1000000;
  quantum.it_value.tv_usec = usec % 1000000;
  /* Activate the virtual timer to generate a SIGVTALRM signal when the quantum is over */
  if(setitimer(ITIMER_VIRTUAL, &quantum, (struct itimerval *)0) == -1){
    perror("setitimer");
    exit(3);
  }
}

/* Initialize the thread library */
void init_mythreadlib() {
  int i;  
  t_state[0].state = INIT;
  t_state[0].priority = LOW_PRIORITY;
  t_state[0].ticks = QUANTUM_TICKS;
  if(getcontext(&t_state[0].run_env) == -1){
    perror("getcontext in my_thread_create");
    exit(5);
  }	
  for(i=1; i<N; i++){
    t_state[i].state = FREE;
  }
  init_interrupt();
  /* Reset the timer */
  reset_timer(QUANTUM_TIME);
}


/* Create and intialize a new thread with body fun_addr and one integer argument */ 
int mythread_create (void (*fun_addr)(),int priority)
{
  int i;
  
  if (!init) { init_mythreadlib(); init=1;}
  for (i=0; i<N; i++)
    if (t_state[i].state == FREE) break;
  if (i == N) return(-1);
  if(getcontext(&t_state[i].run_env) == -1){
    perror("getcontext in my_thread_create");
    exit(-1);
  }
  t_state[i].state = INIT;
  t_state[i].priority = priority;
  t_state[i].function = fun_addr;
  t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));
  if(t_state[i].run_env.uc_stack.ss_sp == NULL){
    printf("thread failed to get stack space\n");
    exit(-1);
  }
  t_state[i].run_env.uc_stack.ss_size = STACKSIZE;
  t_state[i].run_env.uc_stack.ss_flags = 0;
  makecontext(&t_state[i].run_env, fun_addr, 1);  
  return i;
} /****** End my_thread_create() ******/


/* Free terminated thread and exits */
void mythread_exit() {
  int tid = mythread_gettid();	

  printf("Thread %d finished\n ***************\n", tid);	
  t_state[tid].state = FREE;
  free(t_state[tid].run_env.uc_stack.ss_sp); 
  scheduler();
}

/* Sets the priority of the calling thread */
void mythread_setpriority(int priority) {
  int tid = mythread_gettid();	
  t_state[tid].priority = priority;
}

/* Returns the priority of the calling thread */
int mythread_getpriority(int priority) {
  int tid = mythread_gettid();	
  return t_state[tid].priority;
}


/* Get the current thread id.  */
int mythread_gettid(){
  if (!init) { init_mythreadlib(); init=1;}
  return current;
}

/* Timer interrupt  */
void timer_interrupt(int sig)
{
} 

/* Scheduler */
void scheduler(){
  int i;

  for(i=0; i<N; i++){
      if (t_state[i].state == INIT) {
	current = i;
	reset_timer(QUANTUM_TIME);
	setcontext (&(t_state[current].run_env));
	printf("mythread_free: After setcontext, should never get here!!...\n");	
	return;
      }	
  }	
  if (i == N) {
	printf("mythread_free: No thread in the system\nExiting...\n");	
	exit(1);
  }	
}



