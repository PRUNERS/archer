// RUN: %libarcher-compile-and-run
#include <omp.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  int var = 0;

  omp_lock_t lock;
  omp_init_lock(&lock);

  #pragma omp parallel num_threads(2) shared(var)
  {
    omp_set_lock(&lock);
    var++;
    omp_unset_lock(&lock);
  }

  omp_destroy_lock(&lock);

  int error = (var != 2);
  return error;
}
