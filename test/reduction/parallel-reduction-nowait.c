// RUN: %libarcher-compile-and-run
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0, i;
  int sum1 = 0;
  int sum2 = 0;
  
  // Number of threads is empirical: We need enough threads so that
  // the reduction is really performed hierarchically in the barrier!
  #pragma omp parallel num_threads(5) reduction(+: var)
  {
    #pragma omp for schedule(static) nowait reduction(+: sum1)
    for (i=0; i<5; i++)
      sum1+=i;
    #pragma omp for schedule(static) reduction(+: sum2)
    for (i=0; i<5; i++)
      sum2+=i;

    var = sum1 + sum2;
  }

  int error = (var != 100);
  return error;
}
