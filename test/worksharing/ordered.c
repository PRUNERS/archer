// RUN: %libarcher-compile-and-run
#include <omp.h>
#include <stdio.h>

#define NUM_THREADS 2

int main(int argc, char* argv[])
{
  int var = 0;
  int i;

  #pragma omp parallel for ordered num_threads(NUM_THREADS) shared(var)
  for (i = 0; i < NUM_THREADS; i++) {
    #pragma omp ordered
    {
      var++;
    }
  }

  int error = (var != 2);
  return error;
}
