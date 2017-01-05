// RUN: %raceomp-compile-and-run | FileCheck %s
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
    // Dummy locking.
    omp_unset_lock(&lock);

    var++;
  }

  omp_destroy_lock(&lock);

  int error = (var != 2);
  fprintf(stderr, "DONE\n");
  return error;
}

// CHECK: WARNING: ThreadSanitizer: data race
// CHECK:   Write of size 4
// CHECK: #0 .omp_outlined.
// CHECK:   Previous write of size 4
// CHECK: #0 .omp_outlined.
// CHECK: DONE
