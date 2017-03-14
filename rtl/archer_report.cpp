#include <stdio.h>
#include <iostream>
#include <map>

extern "C" 
{
	void __tsan_on_report(void *report);
	void *__tsan_get_current_report();
	int __tsan_get_report_data(void *report, const char **description, int *count,
	                           int *stack_count, int *mop_count, int *loc_count,
	                           int *mutex_count, int *thread_count,
	                           int *unique_tid_count, void **sleep_trace,
	                           unsigned long trace_size);
	int __tsan_get_report_mop(void *report, unsigned long idx, int *tid,
	                          void **addr, int *size, int *write, int *atomic,
	                          void **trace, unsigned long trace_size);
	int __tsan_get_report_thread(void *report, unsigned long idx, int *tid,
	                             unsigned long *os_id, int *running,
	                             const char **name, int *parent_tid, void **trace,
	                             unsigned long trace_size);
}

std::map<void*, void*> addr_map;

void __tsan_on_report(void *report) 
{

	const char *description;
	int count;
	int stack_count, mop_count, loc_count, mutex_count, thread_count,
      unique_tid_count;
	void *sleep_trace[16] = {0};

	__tsan_get_report_data(report, &description, &count, &stack_count, &mop_count,
                         &loc_count, &mutex_count, &thread_count,
                         &unique_tid_count, sleep_trace, 16);

	int tid;
  	void *addr;
  	int size, write, atomic;
  	void *trace[16] = {0};

	__tsan_get_report_mop(report, 0, &tid, &addr, &size, &write, &atomic, trace,
                        16);
	
	auto it = addr_map.find(addr);
	
	if(it->first==NULL)
	{
		addr_map.insert(std::pair<void*, void*>(addr, __tsan_get_current_report()));
		fprintf(stderr, "current_report = %p : Unique\n", __tsan_get_current_report());
	}
	else
	{
  	fprintf(stderr, "current_report = %p : Duplicate of %p\n", __tsan_get_current_report(), it->second);

	}		
}