#include <iostream>

extern "C" void __tsan_on_report(void *rep) 
{
	std::cout<<"Check\n";
}