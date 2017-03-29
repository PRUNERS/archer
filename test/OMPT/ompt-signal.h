// These functions are used to provide a signal-wait mechanism to enforce expected scheduling for the test cases.
// Conditional variable (s) needs to be shared! Initialize to 0
#include <unistd.h>

#define OMPT_SIGNAL(s) ompt_signal(&s)
//inline 
void ompt_signal(int* s) 
{                
  #pragma omp atomic
  (*s)++;
}
                
#define OMPT_WAIT(s,v) ompt_wait(&s,v)
// wait for s == v
//inline 
void ompt_wait(int *s, int v)
{
  int wait;
  do{
    usleep(10);
    #pragma atomic
	  wait = (*s)+0;
  }while(wait!=v);
}
