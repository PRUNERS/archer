#include <iostream>
#include <string>

int main()
{
  int a = 2222;
#pragma omp parallel for
  for (int i = 0; i < 10000; i++) {
    std::cout << a << std::endl;
    std::cout << std::to_string(a) << std::endl;
  }
}
