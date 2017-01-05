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
    #pragma omp task
    {
      #pragma omp task shared(var)
      {
        var++;
      }
    }

    // Give other thread time to steal the task and execute its child.
    sleep(1);

    // Only directly generated children are guaranteed to be executed.
    #pragma omp taskwait
    var++;
  }

  int error = (var != 2);
  fprintf(stderr, "DONE\n");
  return error;
}

// CHECK: WARNING: ThreadSanitizer: data race
// CHECK:   Write of size 4
// CHECK: #0 .omp_outlined.
// CHECK:   Previous write of size 4
// CHECK: #0 .omp_task_entry.
// CHECK: DONE
