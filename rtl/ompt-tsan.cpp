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

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stack>
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <sys/resource.h>
#define _OPENMP
#include "omp.h"
#if !defined(__powerpc64__)
#include <ompt.h>
#endif

callback_counter_t *all_counter;
__thread callback_counter_t* this_event_counter;

class ArcherFlags {
public:
#if (LLVM_VERSION) >= 40
  int flush_shadow;
#endif
  int print_ompt_counters;
  int print_max_rss;

  ArcherFlags(const char *env) :
#if (LLVM_VERSION) >= 40
    flush_shadow(0),
#endif
    print_ompt_counters(0),
    print_max_rss(0) {
    if(env) {
      std::vector<std::string> tokens;
      std::string token;
      std::string str(env);
      std::istringstream iss(str);
      while(std::getline(iss, token, ' '))
        tokens.push_back(token);

      int ret;
      for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
#if (LLVM_VERSION) >= 40
        ret = sscanf(it->c_str(), "flush_shadow=%d", &flush_shadow);
#endif
        ret = sscanf(it->c_str(), "print_ompt_counters=%d", &print_ompt_counters);
        ret = sscanf(it->c_str(), "print_max_rss=%d", &print_max_rss);
        if(ret) {
          std::cerr << "Illegal values for ARCHER_OPTIONS variable: " << token << std::endl;
        }
      }
    }
  }
};

#if (LLVM_VERSION) >= 40
extern "C" {
  int __attribute__((weak)) __archer_get_omp_status();
  void __attribute__((weak)) __tsan_flush_memory() {}
}
#endif
ArcherFlags *archer_flags;

// The following definitions are pasted from "llvm/Support/Compiler.h" to allow the code
// to be compiled with other compilers like gcc:

#ifndef TsanHappensBefore
// Thread Sanitizer is a tool that finds races in code.
// See http://code.google.com/p/data-race-test/wiki/DynamicAnnotations .
// tsan detects these exact functions by name.
extern "C" {
void __attribute__((weak)) AnnotateHappensAfter(const char *file, int line, const volatile void *cv){}
void __attribute__((weak)) AnnotateHappensBefore(const char *file, int line, const volatile void *cv){}
void __attribute__((weak)) AnnotateIgnoreWritesBegin(const char *file, int line){}
void __attribute__((weak)) AnnotateIgnoreWritesEnd(const char *file, int line){}

void __attribute__((weak)) AnnotateNewMemory(const char *file, int line, const volatile void *cv, size_t size){}
}

// This marker is used to define a happens-before arc. The race detector will
// infer an arc from the begin to the end when they share the same pointer
// argument.
# define TsanHappensBefore(cv) AnnotateHappensBefore(__FILE__, __LINE__, cv)

// This marker defines the destination of a happens-before arc.
# define TsanHappensAfter(cv) AnnotateHappensAfter(__FILE__, __LINE__, cv)

// Ignore any races on writes between here and the next TsanIgnoreWritesEnd.
# define TsanIgnoreWritesBegin() AnnotateIgnoreWritesBegin(__FILE__, __LINE__)

// Resume checking for racy writes.
# define TsanIgnoreWritesEnd() AnnotateIgnoreWritesEnd(__FILE__, __LINE__)

// We don't really delete the clock for now
# define TsanDeleteClock(cv)

// newMemory
# define TsanNewMemory(addr, size) AnnotateNewMemory(__FILE__, __LINE__, addr, size)
# define TsanFreeMemory(addr, size) AnnotateNewMemory(__FILE__, __LINE__, addr, size)
#endif


/// Required OMPT inquiry functions.
static ompt_get_parallel_info_t ompt_get_parallel_info;
static ompt_get_thread_data_t ompt_get_thread_data;

typedef int (* ompt_get_task_memory_t) (void** addr, size_t* size, int blocknum);
static ompt_get_task_memory_t ompt_get_task_memory_info;

typedef uint64_t ompt_tsan_clockid;

static uint64_t my_next_id()
{
  static uint64_t ID=0;
  uint64_t ret = __sync_fetch_and_add(&ID,1);
  return ret;
}

// Data structure to provide a threadsafe pool of reusable objects.
// DataPool<Type of objects, Size of blockalloc>
template <typename T, int N>
struct DataPool {
  std::mutex DPMutex;
  std::stack<T *> DataPointer;
  int total;


  void newDatas(){
    // prefix the Data with a pointer to 'this', allows to return memory to 'this',
    // without explicitly knowing the source.
    //
    // To reduce lock contention, we use thread local DataPools, but Data objects move to other threads.
    // The strategy is to get objects from local pool. Only if the object moved to another
    // thread, we might see a penalty on release (returnData).
    // For "single producer" pattern, a single thread creates tasks, these are executed by other threads.
    // The master will have a high demand on TaskData, so return after use.
    struct pooldata {DataPool<T,N>* dp; T data;};
    // We alloc without initialize the memory. We cannot call constructors. Therfore use malloc!
    pooldata* datas = (pooldata*) malloc(sizeof(pooldata) * N);
    for (int i = 0; i<N; i++) {
      datas[i].dp = this;
      DataPointer.push(&(datas[i].data));
    }
    total+=N;
  }

  T * getData() {
    T * ret;
    DPMutex.lock();
    if (DataPointer.empty())
      newDatas();
    ret=DataPointer.top();
    DataPointer.pop();
    DPMutex.unlock();
    return ret;
  }

  void returnData(T * data) {
    DPMutex.lock();
    DataPointer.push(data);
    DPMutex.unlock();
  }

  void getDatas(int n, T** datas) {
    DPMutex.lock();
    for (int i=0; i<n; i++) {
      if (DataPointer.empty())
        newDatas();
      datas[i]=DataPointer.top();
      DataPointer.pop();
    }
    DPMutex.unlock();
  }

  void returnDatas(int n, T** datas) {
    DPMutex.lock();
    for (int i=0; i<n; i++) {
      DataPointer.push(datas[i]);
    }
    DPMutex.unlock();
  }

  DataPool() : DPMutex(), DataPointer(), total(0)
  {}

};

// This function takes care to return the data to the originating DataPool
// A pointer to the originating DataPool is stored just before the actual data.
template <typename T, int N>
  static void retData(void * data) {
    ((DataPool<T,N>**)data)[-1]->returnData((T*)data);
  }

struct ParallelData;
__thread DataPool<ParallelData,4> *pdp;

/// Data structure to store additional information for parallel regions.
struct ParallelData {

// Parallel fork is just another barrier, use Barrier[1]

  /// Two addresses for relationships with barriers.
  ompt_tsan_clockid Barrier[2];

  void *GetParallelPtr() {
    return &(Barrier[1]);
  }

  void *GetBarrierPtr(unsigned Index) {
    return &(Barrier[Index]);
  }

  ~ParallelData(){
    TsanDeleteClock(&(Barrier[0]));
    TsanDeleteClock(&(Barrier[1]));
  }
  // overload new/delete to use DataPool for memory management.
  void * operator new(size_t size){
    return pdp->getData();
  }
  void operator delete(void* p, size_t){
    retData<ParallelData,4>(p);
  }
};

static inline ParallelData *ToParallelData(ompt_data_t* parallel_data) {
  return reinterpret_cast<ParallelData*>(parallel_data->ptr);
}

struct Taskgroup;
__thread DataPool<Taskgroup,4> *tgp;

/// Data structure to support stacking of taskgroups and allow synchronization.
struct Taskgroup {
  /// Its address is used for relationships of the taskgroup's task set.
  ompt_tsan_clockid Ptr;

  /// Reference to the parent taskgroup.
  Taskgroup* Parent;

  Taskgroup(Taskgroup* Parent) : Parent(Parent) {
  }
  ~Taskgroup() {
    TsanDeleteClock(&Ptr);
  }

  void *GetPtr() {
    return &Ptr;
  }
  // overload new/delete to use DataPool for memory management.
  void * operator new(size_t size){
    return tgp->getData();
  }
  void operator delete(void* p, size_t){
    retData<Taskgroup,4>(p);
  }
};

struct TaskData;
__thread DataPool<TaskData,4> *tdp;

/// Data structure to store additional information for tasks.
struct TaskData {
  /// Its address is used for relationships of this task.
  ompt_tsan_clockid Task;

  /// Child tasks use its address to declare a relationship to a taskwait in
  /// this task.
  ompt_tsan_clockid Taskwait;

  /// Whether this task is currently executing a barrier.
  bool InBarrier;

  /// Whether this task is currently executing a barrier.
  bool Included;

  /// Index of which barrier to use next.
  char BarrierIndex;

  /// Count how often this structure has been put into child tasks + 1.
  std::atomic_int RefCount;

  /// Reference to the parent that created this task.
  TaskData* Parent;

  /// Reference to the implicit task in the stack above this task.
  TaskData* ImplicitTask;

  /// Reference to the team of this task.
  ParallelData* Team;

  /// Reference to the current taskgroup that this task either belongs to or
  /// that it just created.
  Taskgroup* TaskGroup;

  /// Dependency information for this task.
  ompt_task_dependence_t* Dependencies;

  /// Number of dependency entries.
  unsigned DependencyCount;

  void* PrivateData;
  size_t PrivateDataSize;

  int execution;
  int freed;

  TaskData(TaskData* Parent) : InBarrier(false), Included(false), BarrierIndex(0),
    RefCount(1), Parent(Parent), ImplicitTask(nullptr), Team(Parent->Team), TaskGroup(nullptr), DependencyCount(0), execution(0), freed(0) {
    if (Parent != nullptr) {
      Parent->RefCount++;
      // Copy over pointer to taskgroup. This task may set up its own stack
      // but for now belongs to its parent's taskgroup.
      TaskGroup = Parent->TaskGroup;
    }
  }

  TaskData(ParallelData* Team = nullptr) : InBarrier(false), Included(false), BarrierIndex(0),
    RefCount(1), Parent(nullptr), ImplicitTask(this), Team(Team), TaskGroup(nullptr), DependencyCount(0), execution(1), freed(0) {
  }

  ~TaskData() {
    TsanDeleteClock(&Task);
    TsanDeleteClock(&Taskwait);
  }

  void *GetTaskPtr() {
    return &Task;
  }

  void *GetTaskwaitPtr() {
    return &Taskwait;
  }
  // overload new/delete to use DataPool for memory management.
  void * operator new(size_t size){
    return tdp->getData();
  }
  void operator delete(void* p, size_t){
    retData<TaskData,4>(p);
  }
};

struct TaskData;
struct ParallelData;
struct Taskgroup;
union OMPTData{
  ParallelData pd;
  Taskgroup tg;
  TaskData td;
};

static inline TaskData *ToTaskData(ompt_data_t *task_data) {
  return reinterpret_cast<TaskData*>(task_data->ptr);
}

static inline void *ToInAddr(void* OutAddr) {
  // FIXME: This will give false negatives when a second variable lays directly
  //        behind a variable that only has a width of 1 byte.
  //        Another approach would be to "negate" the address or to flip the
  //        first bit...
  return reinterpret_cast<char*>(OutAddr) + 1;
}


/// Store a mutex for each wait_id to resolve race condition with callbacks.
std::unordered_map<ompt_wait_id_t, std::mutex> Locks;
std::mutex LocksMutex;

static inline void* ToWaitPtr(ompt_wait_id_t wait_id) {
  // FIXME: wait_ids may be in the same range as "normal" addresses are...
  return reinterpret_cast<void*>(wait_id);
}


static void
ompt_tsan_thread_begin(
  ompt_thread_type_t thread_type,
  ompt_data_t *thread_data)
{
  pdp = new DataPool<ParallelData,4>;
  TsanNewMemory(pdp, sizeof(pdp));
  tgp = new DataPool<Taskgroup,4>;
  TsanNewMemory(tgp, sizeof(tgp));
  tdp = new DataPool<TaskData,4>;
  TsanNewMemory(tdp, sizeof(tdp));
  thread_data->value = my_next_id();
  if(archer_flags->print_ompt_counters && thread_data->value<MAX_THREADS)
    this_event_counter = &(all_counter[thread_data->value]);
  else
    this_event_counter=NULL;
  COUNT_EVENT1(thread_begin);
}

/*static void
ompt_tsan_thread_end(
  ompt_data_t *thread_data)
{
  printf("%lu: total PD: %lu / %i TD: %lu / %i TG: %lu / %i\n", thread_data->value, pdp->total - pdp->DataPointer.size(), pdp->total, tdp->total - tdp->DataPointer.size(),
    tdp->total, tgp->total - tgp->DataPointer.size(), tgp->total);
  COUNT_EVENT1(thread_end);
}*/

/// OMPT event callbacks for handling parallel regions.

static void
ompt_tsan_parallel_begin(
  ompt_data_t *parent_task_data,
  const ompt_frame_t *parent_task_frame,
  ompt_data_t* parallel_data,
  uint32_t requested_team_size,
//  uint32_t actual_team_size,
  ompt_invoker_t invoker,
  const void *codeptr_ra)
{
  ParallelData* Data = new ParallelData;
  parallel_data->ptr = Data;

  TsanHappensBefore(Data->GetParallelPtr());
  COUNT_EVENT1(parallel_begin);
}

static void
ompt_tsan_parallel_end(
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  ompt_invoker_t invoker,
  const void *codeptr_ra)
{
  ParallelData* Data = ToParallelData(parallel_data);
  TsanHappensAfter(Data->GetBarrierPtr(0));
  TsanHappensAfter(Data->GetBarrierPtr(1));

  delete Data;

#if (LLVM_VERSION >= 40)
  if(&__archer_get_omp_status) {
    if(__archer_get_omp_status() == 0 && archer_flags->flush_shadow)
      __tsan_flush_memory();
  }
#endif

  COUNT_EVENT1(parallel_end);
}

static void
ompt_tsan_implicit_task(
    ompt_scope_endpoint_t endpoint,
    ompt_data_t *parallel_data,
    ompt_data_t *task_data,
    unsigned int team_size,
    unsigned int thread_num)
{
  switch(endpoint)
  {
     case ompt_scope_begin:
        task_data->ptr = new TaskData(ToParallelData(parallel_data));
        TsanHappensAfter(ToParallelData(parallel_data)->GetParallelPtr());
        COUNT_EVENT2(implicit_task,scope_begin);
        break;
     case ompt_scope_end:
        TaskData* Data = ToTaskData(task_data);
        assert(Data->freed == 0 && "Implicit task end should only be called once!");
        Data->freed=1;
        assert(Data->RefCount == 1 && "All tasks should have finished at the implicit barrier!");
        delete Data;
        COUNT_EVENT2(implicit_task,scope_end);
        break;
  }
}

static void
ompt_tsan_sync_region(
  ompt_sync_region_kind_t kind,
  ompt_scope_endpoint_t endpoint,
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  const void *codeptr_ra)
{
  TaskData* Data = ToTaskData(task_data);
  switch(endpoint)
  {
    case ompt_scope_begin:
      switch(kind)
      {
        case ompt_sync_region_barrier:
          {
            char BarrierIndex = Data->BarrierIndex;
            TsanHappensBefore(Data->Team->GetBarrierPtr(BarrierIndex));

            // We ignore writes inside the barrier. These would either occur during
            // 1. reductions performed by the runtime which are guaranteed to be race-free.
            // 2. execution of another task.
            // For the latter case we will re-enable tracking in task_switch.
            Data->InBarrier = true;
            TsanIgnoreWritesBegin();
            COUNT_EVENT3(sync_region,scope_begin,barrier);
            break;
          }
        case ompt_sync_region_taskwait:

            COUNT_EVENT3(sync_region,scope_begin,taskwait);
            break;
        case ompt_sync_region_taskgroup:
            Data->TaskGroup = new Taskgroup(Data->TaskGroup);
            COUNT_EVENT3(sync_region,scope_begin,taskgroup);
            break;
      }
      break;
    case ompt_scope_end:
      switch(kind)
      {
        case ompt_sync_region_barrier:
          {
            // We want to track writes after the barrier again.
            Data->InBarrier = false;
            TsanIgnoreWritesEnd();

            char BarrierIndex = Data->BarrierIndex;
            // Barrier will end after it has been entered by all threads.
            if (parallel_data)
              TsanHappensAfter(Data->Team->GetBarrierPtr(BarrierIndex));

            // It is not guaranteed that all threads have exited this barrier before
            // we enter the next one. So we will use a different address.
            // We are however guaranteed that this current barrier is finished
            // by the time we exit the next one. So we can then reuse the first address.
            Data->BarrierIndex = (BarrierIndex + 1) % 2;
            COUNT_EVENT3(sync_region,scope_end,barrier);
            break;
          }
        case ompt_sync_region_taskwait:
          {
            COUNT_EVENT3(sync_region,scope_end,taskwait);
            if(Data->execution>1)
              TsanHappensAfter(Data->GetTaskwaitPtr());
            break;
          }
        case ompt_sync_region_taskgroup:
          {
            assert(Data->TaskGroup != nullptr && "Should have at least one taskgroup!");

            TsanHappensAfter(Data->TaskGroup->GetPtr());

            // Delete this allocated taskgroup, all descendent task are finished by now.
            Taskgroup* Parent = Data->TaskGroup->Parent;
            delete Data->TaskGroup;
            Data->TaskGroup = Parent;
            COUNT_EVENT3(sync_region,scope_end,taskgroup);
            break;
          }
      }
      break;
  }
}



/// OMPT event callbacks for handling tasks.

static void
ompt_tsan_task_create(
    ompt_data_t *parent_task_data,    /* id of parent task            */
    const ompt_frame_t *parent_frame,  /* frame data for parent task   */
    ompt_data_t* new_task_data,      /* id of created task           */
    int type,
    int has_dependences,
    const void *codeptr_ra)               /* pointer to outlined function */
{
  TaskData* Data;
  assert(new_task_data->ptr == NULL && "Task data should be initialized to NULL");
  if (type & ompt_task_initial)
  {
    ompt_data_t* parallel_data;
    int team_size = 1;
    ompt_get_parallel_info(0, &parallel_data, &team_size);
    ParallelData* PData = new ParallelData;
    parallel_data->ptr = PData;

    Data = new TaskData(PData);
    new_task_data->ptr = Data;
    COUNT_EVENT2(task_create,initial);
  } else if (type & ompt_task_undeferred) {
    Data = new TaskData(ToTaskData(parent_task_data));
    new_task_data->ptr = Data;
    Data->Included=true;
    COUNT_EVENT2(task_create,included);
  } else if (type & ompt_task_explicit || type & ompt_task_target) {
    Data = new TaskData(ToTaskData(parent_task_data));
    new_task_data->ptr = Data;

    // Use the newly created address. We cannot use a single address from the
    // parent because that would declare wrong relationships with other
    // sibling tasks that may be created before this task is started!
    TsanHappensBefore(Data->GetTaskPtr());
    ToTaskData(parent_task_data)->execution++;
    COUNT_EVENT2(task_create,explicit);
  }
}

static void
ompt_tsan_task_schedule(
    ompt_data_t *first_task_data,
    ompt_task_status_t prior_task_status,
    ompt_data_t *second_task_data)
{
  COUNT_EVENT1(task_schedule);
  TaskData* FromTask = ToTaskData(first_task_data);
  TaskData* ToTask = ToTaskData(second_task_data);

  if (ToTask->Included && prior_task_status != ompt_task_complete)
    return; // No further synchronization for begin included tasks
  if (FromTask->Included && prior_task_status == ompt_task_complete) {
    // Just delete the task:
    while (FromTask != nullptr && --FromTask->RefCount == 0) {
      TaskData* Parent = FromTask->Parent;
      if (FromTask->DependencyCount > 0) {
        delete[] FromTask->Dependencies;
      }
      delete FromTask;
      FromTask = Parent;
    }
    return;
  }

  // we will use new stack, assume growing down
  // TsanNewMemory((char*)&FromTask-1024, 1024);
  if (ToTask->execution==0) {
    ToTask->execution++;
  // 1. Task will begin execution after it has been created.
    TsanHappensAfter(ToTask->GetTaskPtr());
    if ( ompt_get_task_memory_info ) {
      if ( !ompt_get_task_memory_info( &(ToTask->PrivateData), &(ToTask->PrivateDataSize), 0) ) {
        ToTask->PrivateData=nullptr; ToTask->PrivateDataSize=0;
      } else {
//        printf("TsanNewMemory(%p, %lu)\n", ToTask->PrivateData, ToTask->PrivateDataSize);
        TsanNewMemory(ToTask->PrivateData, ToTask->PrivateDataSize);
      }
    }
    for (unsigned i = 0; i < ToTask->DependencyCount; i++) {
      ompt_task_dependence_t* Dependency = &ToTask->Dependencies[i];

      TsanHappensAfter(Dependency->variable_addr);
      // in and inout dependencies are also blocked by prior in dependencies!
      if (Dependency->dependence_flags & ompt_task_dependence_type_out) {
        TsanHappensAfter(ToInAddr(Dependency->variable_addr));
      }
    }
  } else {
  // 2. Task will resume after it has been switched away.
    TsanHappensAfter(ToTask->GetTaskPtr());
  }

  if (prior_task_status != ompt_task_complete) {
    ToTask->ImplicitTask = FromTask->ImplicitTask;
    assert(ToTask->ImplicitTask != NULL && "A task belongs to a team and has an implicit task on the stack");
  }

  // Task may be resumed at a later point in time.
  //TsanHappensBeforeUC(FromTask->GetTaskPtr());
  TsanHappensBefore(FromTask->GetTaskPtr());

  if (FromTask->InBarrier) {
    // We want to ignore writes in the runtime code during barriers,
    // but not when executing tasks with user code!
    TsanIgnoreWritesEnd();
  }

  if (prior_task_status == ompt_task_complete) { // task finished
    if ( ompt_get_task_memory_info && FromTask->PrivateDataSize) {
//          printf("TsanFreeMemory(%p, %p, %lu)\n", FromTask->PrivateData, (char*)FromTask->PrivateData+FromTask->PrivateDataSize, FromTask->PrivateDataSize);
      TsanFreeMemory(FromTask->PrivateData, FromTask->PrivateDataSize);
    }

    // Task will finish before a barrier in the surrounding parallel region ...
    ParallelData* PData = FromTask->Team;
    TsanHappensBefore(PData->GetBarrierPtr(FromTask->ImplicitTask->BarrierIndex));

    // ... and before an eventual taskwait by the parent thread.
    TsanHappensBefore(FromTask->Parent->GetTaskwaitPtr());

    if (FromTask->TaskGroup != nullptr) {
        // This task is part of a taskgroup, so it will finish before the
        // corresponding taskgroup_end.
        TsanHappensBefore(FromTask->TaskGroup->GetPtr());
    }
    for (unsigned i = 0; i < FromTask->DependencyCount; i++) {
        ompt_task_dependence_t* Dependency = &FromTask->Dependencies[i];

        // in dependencies block following inout and out dependencies!
        TsanHappensBefore(ToInAddr(Dependency->variable_addr));
        if (Dependency->dependence_flags & ompt_task_dependence_type_out) {
            TsanHappensBefore(Dependency->variable_addr);
        }
    }
    while (FromTask != nullptr && --FromTask->RefCount == 0) {
        TaskData* Parent = FromTask->Parent;
        if (FromTask->DependencyCount > 0) {
            delete[] FromTask->Dependencies;
        }
        delete FromTask;
        FromTask = Parent;
    }
  }
  if (ToTask->InBarrier) {
    // We re-enter runtime code which currently performs a barrier.
    TsanIgnoreWritesBegin();
  }

}

static void ompt_tsan_task_dependences(
  ompt_data_t* task_data,
  const ompt_task_dependence_t *deps,
  int ndeps)
{
  COUNT_EVENT1(task_dependences);
  if (ndeps > 0) {
    // Copy the data to use it in task_switch and task_end.
    TaskData* Data = ToTaskData(task_data);
    Data->Dependencies = new ompt_task_dependence_t[ndeps];
    std::memcpy(Data->Dependencies, deps, sizeof(ompt_task_dependence_t) * ndeps);
    Data->DependencyCount = ndeps;

    // This callback is executed before this task is first started.
    TsanHappensBefore(Data->GetTaskPtr());
  }
}

/// OMPT event callbacks for handling locking.
static void ompt_tsan_mutex_acquired(
  ompt_mutex_kind_t kind,
  ompt_wait_id_t wait_id,
  const void *codeptr_ra)
{
  if(archer_flags->print_ompt_counters)
    switch(kind)
    {
      case ompt_mutex_lock:
        COUNT_EVENT2(mutex_acquired, lock);
        break;
      case ompt_mutex_nest_lock:
        COUNT_EVENT2(mutex_acquired, nest_lock);
        break;
      case ompt_mutex_critical:
        COUNT_EVENT2(mutex_acquired, critical);
        break;
      case ompt_mutex_atomic:
        COUNT_EVENT2(mutex_acquired, atomic);
        break;
      case ompt_mutex_ordered:
        COUNT_EVENT2(mutex_acquired, ordered);
        break;
      default:
        COUNT_EVENT2(mutex_acquired, default);
        break;
    }

  // Acquire our own lock to make sure that
  // 1. the previous release has finished.
  // 2. the next acquire doesn't start before we have finished our release.
  {
    LocksMutex.lock();
    std::mutex& Lock = Locks[wait_id];
    LocksMutex.unlock();

    Lock.lock();
  }

  TsanHappensAfter(ToWaitPtr(wait_id));
}

static void ompt_tsan_mutex_released(
  ompt_mutex_kind_t kind,
  ompt_wait_id_t wait_id,
  const void *codeptr_ra)
{
  if(archer_flags->print_ompt_counters)
    switch(kind)
    {
      case ompt_mutex_lock:
        COUNT_EVENT2(mutex_released, lock);
        break;
      case ompt_mutex_nest_lock:
        COUNT_EVENT2(mutex_released, nest_lock);
        break;
      case ompt_mutex_critical:
        COUNT_EVENT2(mutex_released, critical);
        break;
      case ompt_mutex_atomic:
        COUNT_EVENT2(mutex_released, atomic);
        break;
      case ompt_mutex_ordered:
        COUNT_EVENT2(mutex_released, ordered);
        break;
      default:
        COUNT_EVENT2(mutex_released, default);
        break;
    }
  TsanHappensBefore(ToWaitPtr(wait_id));

  {
    LocksMutex.lock();
    std::mutex& Lock = Locks[wait_id];
    LocksMutex.unlock();

    Lock.unlock();
  }
}

#define SET_CALLBACK_T(event, type)                           \
do{                                                           \
  ompt_callback_##type##_t tsan_##event = &ompt_tsan_##event; \
  int ret = ompt_set_callback(ompt_callback_##event,          \
      (ompt_callback_t) tsan_##event);                        \
  if (ret != ompt_set_always)                                 \
      printf("Registered callback '" #event                   \
            "' is not always invoked (%i)\n", ret);           \
}while(0)

#define SET_CALLBACK(event) SET_CALLBACK_T(event, event)


static int ompt_tsan_initialize(
  ompt_function_lookup_t lookup,
  ompt_data_t *tool_data
  ) {

  const char *options = getenv("ARCHER_OPTIONS");
  archer_flags = new ArcherFlags(options);

  if(archer_flags->print_ompt_counters)
    all_counter = new callback_counter_t[MAX_THREADS];

  ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
  if (ompt_set_callback == NULL) {
    std::cerr << "Could not set callback, exiting..." << std::endl;
    std::exit(1);
  }
  ompt_get_parallel_info = (ompt_get_parallel_info_t) lookup("ompt_get_parallel_info");
  ompt_get_thread_data = (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
  ompt_get_task_memory_info = nullptr;
  ompt_get_task_memory_info = (ompt_get_task_memory_t) lookup("ompt_get_task_memory_info");

  if (ompt_get_parallel_info == NULL) {
    fprintf(stderr, "Could not get inquiry function 'ompt_get_parallel_info', exiting...\n");
    exit(1);
  }

  SET_CALLBACK(thread_begin);
//  SET_CALLBACK(thread_end);
  SET_CALLBACK(parallel_begin);
  SET_CALLBACK(implicit_task);
  SET_CALLBACK(sync_region);
  SET_CALLBACK(parallel_end);

  SET_CALLBACK(task_create);
  SET_CALLBACK(task_schedule);
  SET_CALLBACK(task_dependences);

  SET_CALLBACK_T(mutex_acquired, mutex);
  SET_CALLBACK_T(mutex_released, mutex);
  return 1; // success
}


static void ompt_tsan_finalize(ompt_data_t *tool_data)
{
  if(archer_flags->print_ompt_counters) {
    print_callbacks(all_counter);
    delete[] all_counter;
  }

  if(archer_flags->print_max_rss) {
    struct rusage end;
    getrusage(RUSAGE_SELF, &end);
    printf("MAX RSS[KBytes] during execution: %ld\n", end.ru_maxrss);
  }

  if(archer_flags)
    delete archer_flags;
}

ompt_start_tool_result_t* ompt_start_tool(
  unsigned int omp_version,
  const char *runtime_version)
{
  static ompt_start_tool_result_t ompt_start_tool_result = {&ompt_tsan_initialize,&ompt_tsan_finalize, {0}};
  return &ompt_start_tool_result;
}
