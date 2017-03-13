#include "counter.h"

#define OUTPUT_IF_NOT_NULL(format,value) if (value) printf(format, value)

void print_callbacks(callback_counter_t *counter){
    int* basecounter = (int*)counter;
    int total_callbacks;
    for(int i = 1; i<MAX_THREADS; i++){
        // if we have at least i threads, this thread should have a thread_begin event:
        if (counter[i].thread_begin==0) break;
        int* threadcounter = (int*)(counter+i);
        for(int j=0; j<sizeof(callback_counter_t)/sizeof(int); j++)
            basecounter[j] += threadcounter[j];
    }

    for(int j=0; j<sizeof(callback_counter_t)/sizeof(int); j++)
        total_callbacks += basecounter[j];

    printf("Total callbacks: %d\n", total_callbacks);
    printf("--------------------------------------\n");
    OUTPUT_IF_NOT_NULL("%5d thread_begin\n", counter[0].thread_begin);
    OUTPUT_IF_NOT_NULL("%5d thread_end\n", counter[0].thread_end);
    OUTPUT_IF_NOT_NULL("%5d parallel_begin\n", counter[0].parallel_begin);
    OUTPUT_IF_NOT_NULL("%5d parallel_end\n", counter[0].parallel_end);
    OUTPUT_IF_NOT_NULL("%5d task_create : initial\n", counter[0].task_create_initial);
    OUTPUT_IF_NOT_NULL("%5d task_create : explicit\n", counter[0].task_create_explicit);
    OUTPUT_IF_NOT_NULL("%5d task_create : target\n", counter[0].task_create_target);
    OUTPUT_IF_NOT_NULL("%5d task_create : included\n", counter[0].task_create_included);
    OUTPUT_IF_NOT_NULL("%5d task_create : untied\n", counter[0].task_create_untied);
    OUTPUT_IF_NOT_NULL("%5d task_schedule\n", counter[0].task_schedule);
    OUTPUT_IF_NOT_NULL("%5d implicit_task : scope_begin\n", counter[0].implicit_task_scope_begin);
    OUTPUT_IF_NOT_NULL("%5d implicit_task : scope_end\n", counter[0].implicit_task_scope_end);
    OUTPUT_IF_NOT_NULL("%5d mutex_released_lock\n", counter[0].mutex_released_lock);
    OUTPUT_IF_NOT_NULL("%5d mutex_released_nest_lock\n", counter[0].mutex_released_nest_lock);
    OUTPUT_IF_NOT_NULL("%5d mutex_released_critical\n", counter[0].mutex_released_critical);
    OUTPUT_IF_NOT_NULL("%5d mutex_released_atomic\n", counter[0].mutex_released_atomic);
    OUTPUT_IF_NOT_NULL("%5d mutex_released_ordered\n", counter[0].mutex_released_ordered);
    OUTPUT_IF_NOT_NULL("%5d mutex_released_default\n", counter[0].mutex_released_default);
    OUTPUT_IF_NOT_NULL("%5d task_dependences\n", counter[0].task_dependences);
    OUTPUT_IF_NOT_NULL("%5d task_dependence\n", counter[0].task_dependence);
    OUTPUT_IF_NOT_NULL("%5d sync_region : scope_begin : barrier\n", counter[0].sync_region_scope_begin_barrier);
    OUTPUT_IF_NOT_NULL("%5d sync_region : scope_begin : taskwait\n", counter[0].sync_region_scope_begin_taskwait);
    OUTPUT_IF_NOT_NULL("%5d sync_region : scope_begin : taskgroup\n", counter[0].sync_region_scope_begin_taskgroup);
    OUTPUT_IF_NOT_NULL("%5d sync_region : scope_end : barrier\n", counter[0].sync_region_scope_end_barrier);
    OUTPUT_IF_NOT_NULL("%5d sync_region : scope_end : taskwait\n", counter[0].sync_region_scope_end_taskwait);
    OUTPUT_IF_NOT_NULL("%5d sync_region : scope_end : taskgroup\n", counter[0].sync_region_scope_end_taskgroup);
    OUTPUT_IF_NOT_NULL("%5d lock_init_lock\n", counter[0].lock_init_lock);
    OUTPUT_IF_NOT_NULL("%5d lock_init_nest_lock\n", counter[0].lock_init_nest_lock);
    OUTPUT_IF_NOT_NULL("%5d lock_init_default\n", counter[0].lock_init_default);
    OUTPUT_IF_NOT_NULL("%5d lock_destroy_lock\n", counter[0].lock_destroy_lock);
    OUTPUT_IF_NOT_NULL("%5d lock_destroy_nest_lock\n", counter[0].lock_destroy_nest_lock);
    OUTPUT_IF_NOT_NULL("%5d lock_destroy_default\n", counter[0].lock_destroy_default);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquire_lock\n", counter[0].mutex_acquire_lock);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquire_nest_lock\n", counter[0].mutex_acquire_nest_lock);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquire_critical\n", counter[0].mutex_acquire_critical);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquire_atomic\n", counter[0].mutex_acquire_atomic);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquire_ordered\n", counter[0].mutex_acquire_ordered);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquire_default\n", counter[0].mutex_acquire_default);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquired_lock\n", counter[0].mutex_acquired_lock);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquired_nest_lock\n", counter[0].mutex_acquired_nest_lock);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquired_critical\n", counter[0].mutex_acquired_critical);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquired_atomic\n", counter[0].mutex_acquired_atomic);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquired_ordered\n", counter[0].mutex_acquired_ordered);
    OUTPUT_IF_NOT_NULL("%5d mutex_acquired_default\n", counter[0].mutex_acquired_default);
    OUTPUT_IF_NOT_NULL("%5d nest_lock_scope_begin\n", counter[0].nest_lock_scope_begin);
    OUTPUT_IF_NOT_NULL("%5d nest_lock_scope_end\n", counter[0].nest_lock_scope_end);
    OUTPUT_IF_NOT_NULL("%5d flush\n", counter[0].flush);

    return;
}
