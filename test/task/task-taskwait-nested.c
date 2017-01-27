// RUN: %libarcher-compile-and-run
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
      #pragma omp taskwait
    }

    // Give other thread time to steal the task and execute its child.
    sleep(1);

    #pragma omp taskwait
    var++;
  }

  int error = (var != 2);
  return error;
}
