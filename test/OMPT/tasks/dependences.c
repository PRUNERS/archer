// RUN: %libomp-compile-and-run | %sort-threads | FileCheck %s
// REQUIRES: ompt
#include "callback.h"
#include <omp.h>   
#include <math.h>
#include <unistd.h>

int main()
{
  int x = 0;
  #pragma omp parallel num_threads(2)
  {
    #pragma omp single
    {  
      #pragma omp task depend(out:x)
      {
        x++;
        usleep(100);
      }
    
      #pragma omp task depend(in:x)
      {
        x = -1;
      }
    }
  }

  printf("x = %d\n", x);


  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_create'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_dependences'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_task_dependence'
  
  // CHECK: {{^}}0: NULL_POINTER=[[NULL:.*$]]

  // CHECK: {{^}}{{[0-9]+}}: ompt_event_task_create: parent_task_id={{[0-9]+}}, new_task_id=[[FIRST_TASK:[0-f]+]], parallel_function={{0x[0-f]+}}, task_type=ompt_task_explicit=3, has_dependences=yes
  // CHECK: {{^}}{{[0-9]+}}: ompt_event_task_dependences: task_id=[[FIRST_TASK]], deps={{0x[0-f]+}}, ndeps=1

  // CHECK: {{^}}{{[0-9]+}}: ompt_event_task_create: parent_task_id={{[0-9]+}}, new_task_id=[[SECOND_TASK:[0-f]+]], parallel_function={{0x[0-f]+}}, task_type=ompt_task_explicit=3, has_dependences=yes
  // CHECK: {{^}}{{[0-9]+}}: ompt_event_task_dependences: task_id=[[SECOND_TASK]], deps={{0x[0-f]+}}, ndeps=1
  // CHECK: {{^}}{{[0-9]+}}: ompt_event_task_dependence_pair: first_task_id=[[FIRST_TASK]], second_task_id=[[SECOND_TASK]]


  return 0;
}
