// RUN: %libarcher-compile-and-run
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  // Create team of threads so that there is no implicit happens before
  // when creating the thread.
  #pragma omp parallel num_threads(2)
  { }

  var++;

  #pragma omp parallel num_threads(2) shared(var)
  {
    if (omp_get_thread_num() == 1) {
      var++;
    }
  } // implicit barrier

  int error = (var != 2);
  return error;
}
