// RUN: %libomp-compile-and-run | FileCheck %s
// REQUIRES: ompt
#include "callback.h"

int main()
{
  #pragma omp parallel num_threads(1)
  {
    #pragma omp parallel num_threads(1)
    {
      #pragma omp task
      {
      }
    }
  }

  // Check if libomp supports the callbacks for this test.
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_begin'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_callback_parallel_end'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_event_implicit_task_begin'
  // CHECK-NOT: {{^}}0: Could not register callback 'ompt_event_implicit_task_end'

  // CHECK: 0: NULL_POINTER=[[NULL:.*$]]
  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_parallel_begin: parent_task_id=[[PARENT_TASK_ID:[0-9]+]], parallel_id=[[PARALLEL_ID:[0-9]+]], requested_team_size=1

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_begin: parallel_id=[[PARALLEL_ID]], task_id=[[IMPLICIT_TASK_ID:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_parallel_begin: parent_task_id=[[IMPLICIT_TASK_ID]], parallel_id=[[NESTED_PARALLEL_ID:[0-9]+]], requested_team_size=1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_begin: parallel_id=[[NESTED_PARALLEL_ID]], task_id=[[NESTED_IMPLICIT_TASK_ID:[0-9]+]]

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_create: parent_task_id=[[NESTED_IMPLICIT_TASK_ID]], new_task_id=[[EXPLICIT_TASK_ID:[0-9]+]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_schedule: first_task_id=[[NESTED_IMPLICIT_TASK_ID]], second_task_id=[[EXPLICIT_TASK_ID]], prior_task_status=ompt_task_others=4
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_schedule: first_task_id=[[EXPLICIT_TASK_ID]], second_task_id=[[NESTED_IMPLICIT_TASK_ID]], prior_task_status=ompt_task_complete=1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_task_end: task_id=[[EXPLICIT_TASK_ID]]


  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_end: parallel_id=[[NESTED_PARALLEL_ID]], task_id=[[NESTED_IMPLICIT_TASK_ID]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_parallel_end: parallel_id=[[NESTED_PARALLEL_ID]], task_id=[[IMPLICIT_TASK_ID]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_implicit_task_end: parallel_id=[[PARALLEL_ID]], task_id=[[IMPLICIT_TASK_ID]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_parallel_end: parallel_id=[[PARALLEL_ID]], task_id=[[PARENT_TASK_ID]]

  return 0;
}
