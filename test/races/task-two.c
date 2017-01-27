// RUN: %raceomp-compile-and-run | FileCheck %s
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 2

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(NUM_THREADS) shared(var)
  #pragma omp master
  {
    int i;
    for (i = 0; i < NUM_THREADS; i++) {
      #pragma omp task shared(var)
      {
        var++;
        // Sleep so that each thread executes one single task.
        sleep(1);
      }
    }
  }

  int error = (var != 2);
  fprintf(stderr, "DONE\n");
  return error;
}

// CHECK: WARNING: ThreadSanitizer: data race
// CHECK:   Write of size 4
// CHECK: #0 .omp_task_entry.
// CHECK:   Previous write of size 4
// CHECK: #0 .omp_task_entry.
// CHECK: DONE
