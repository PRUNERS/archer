// RUN: %libarcher-compile-and-run
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  #pragma omp parallel num_threads(2) shared(var)
  {
    if (omp_get_thread_num() == 0) {
      var++;
    }

    #pragma omp barrier

    if (omp_get_thread_num() == 1) {
      var++;
    }
  }

  int error = (var != 2);
  return error;
}
