#include <stdio.h>

extern "C" void __tsan_on_report(void *rep) {
  printf("Test\n");
}
