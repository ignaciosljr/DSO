#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

static sigset_t maskval_interrupt,oldmask_interrupt;

void enable_interrupt(){
  sigprocmask(SIG_SETMASK, &oldmask_interrupt, NULL);
}

void disable_interrupt(){
  sigaddset(&maskval_interrupt, SIGVTALRM);
  sigprocmask(SIG_BLOCK, &maskval_interrupt, &oldmask_interrupt);
}

void init_interrupt()
{
  void timer_interrupt(int sig);
  struct sigaction sigdat;
  /* Initializes the signal mask to empty */
  sigemptyset(&maskval_interrupt); 
  /* Prepare a virtual time alarm */
  sigdat.sa_handler = timer_interrupt;
  sigemptyset(&sigdat.sa_mask);
  sigdat.sa_flags = SA_RESTART;
  if(sigaction(SIGVTALRM, &sigdat, (struct sigaction *)0) == -1){
    perror("signal set error");
    exit(2);
  }
}
