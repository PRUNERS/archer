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
    #pragma omp taskgroup
    {
      #pragma omp task shared(var)
      {
        var++;
      }

      // Give other thread time to steal the task.
      sleep(1);
    }

    var++;
  }

  int error = (var != 2);
  return error;
}
