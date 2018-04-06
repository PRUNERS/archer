/*
Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.

Produced at the Lawrence Livermore National Laboratory

Written by Simone Atzeni (simone@cs.utah.edu), Joachim Protze
(joachim.protze@tu-dresden.de), Jonas Hahnfeld
(hahnfeld@itc.rwth-aachen.de), Ganesh Gopalakrishnan, Zvonimir
Rakamaric, Dong H. Ahn, Gregory L. Lee, Ignacio Laguna, and Martin
Schulz.

LLNL-CODE-727057

All rights reserved.

This file is part of Archer. For details, see
https://pruners.github.io/archer. Please also read
https://github.com/PRUNERS/archer/blob/master/LICENSE.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the disclaimer below.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the disclaimer (as noted below)
   in the documentation and/or other materials provided with the
   distribution.

   Neither the name of the LLNS/LLNL nor the names of its contributors
   may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE
LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "counter.h"

#define OUTPUT_IF_NOT_NULL(format,value) if (value) printf(format, value)

void print_callbacks(callback_counter_t *counter){
    int* basecounter = (int*)counter;
    int total_callbacks=0;
    for(int i = 1; i<MAX_THREADS; i++){
        // if we have at least i threads, this thread should have a thread_begin event:
        if (counter[i].thread_begin==0) break;
        int* threadcounter = (int*)(counter+i);
        for(int j=0; j<(int)(sizeof(callback_counter_t)/sizeof(int)); j++)
            basecounter[j] += threadcounter[j];
    }

    for(int j=0; j<(int)(sizeof(callback_counter_t)/sizeof(int)); j++)
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
