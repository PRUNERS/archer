// RUN: %raceomp-compile-and-run | FileCheck %s
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(2) shared(var)
  #pragma omp master
  {
    #pragma omp task shared(var) depend(out: var)
    {
      var++;
    }

    #pragma omp task depend(in: var)
    {
      // FIXME: This is ugly to make the worker task sleep and let the master
      //        thread execute the second task incrementing var...
      sleep(2);
    }

    #pragma omp task shared(var) // depend(in: var) is missing here!
    {
      var++;
    }

    // Give other thread time to steal the task.
    sleep(1);
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
