#include <cstring>
#include <iostream>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <atomic>
#include <cassert>
#include <mutex>
//#include <shared_mutex>
#include <unordered_map>

#define OMPT_TSAN_MANAGE_CLOCK

#ifndef OMPT_TSAN_MANAGE_CLOCK
#define __SANITIZE_THREAD__
#include "llvm/Support/Compiler.h"
#endif

#include <ompt.h>

#if defined(OMPT_TSAN_CLEANUP_ATOMIC) && ! defined(OMPT_TSAN_CLEANUP)
  #define OMPT_TSAN_CLEANUP
#endif

#if defined(OMPT_TSAN_CLEANUP_ATOMIC) || defined(OMPT_TSAN_CLEANUP)
#include <set>
extern "C" {
void AnnotateDeleteClock(const char *file, int line, const volatile void *cv);
//void AnnotateRWLockDestroy(const char *file, int line, const volatile void *cv);

#define TsanDeleteClock(cv) AnnotateDeleteClock(__FILE__, __LINE__, cv)
//#define TsanDeleteClock(cv) AnnotateRWLockDestroy(__FILE__, __LINE__, cv)
}
#endif

#if defined(OMPT_TSAN_MANAGE_CLOCK) && ! defined(OMPT_TSAN_CLEANUP)
  #define OMPT_TSAN_CLEANUP
#endif

/// Required OMPT inquiry functions.
static ompt_get_parallel_info_t ompt_get_parallel_info;
static ompt_get_thread_data_t ompt_get_thread_data;

static uint64_t my_next_id()
{
  static uint64_t ID=0;
  uint64_t ret = __sync_fetch_and_add(&ID,1);
  return ret;
}
        
#ifdef OMPT_TSAN_MANAGE_CLOCK
#include <set>
 typedef struct SyncClock ompt_tsan_clockid;
extern "C" {
 #define TsanHappensBefore(clock) ((SyncClock*)clock)->HappensBefore()
 #define TsanHappensAfter(clock) ((SyncClock*)clock)->HappensAfter()
 #define TsanDeleteClock(clock) 

void AnnotateIgnoreWritesBegin(const char *file, int line);
void AnnotateIgnoreWritesEnd(const char *file, int line);
// Ignore any races on writes between here and the next TsanIgnoreWritesEnd.
# define TsanIgnoreWritesBegin() AnnotateIgnoreWritesBegin(__FILE__, __LINE__)

// Resume checking for racy writes.
# define TsanIgnoreWritesEnd() AnnotateIgnoreWritesEnd(__FILE__, __LINE__)

void AnnotateBeforeClock(const char *file, int line, volatile void **cv);
#define TsanBeforeClock(cv) AnnotateBeforeClock(__FILE__, __LINE__, cv)
void AnnotateAfterClock(const char *file, int line, volatile void **cv);
#define TsanAfterClock(cv) AnnotateAfterClock(__FILE__, __LINE__, cv)
void AnnotateDeleteClock(const char *file, int line, volatile void **cv);
#define TsanDestroyClock(cv) AnnotateDeleteClock(__FILE__, __LINE__, cv)
}//extern C
#elif defined(OMPT_TSAN_CLEANUP_ATOMIC)
 typedef std::atomic_char ompt_tsan_clockid;
#else
 typedef char ompt_tsan_clockid;
#endif

static inline void *ToInAddr(void* OutAddr) {
  // FIXME: This will give false negatives when a second variable lays directly
  //        behind a variable that only has a width of 1 byte.
  //        Another approach would be to "negate" the address or to flip the
  //        first bit...
  return reinterpret_cast<char*>(OutAddr) + 1;
}

struct SyncClock {
  volatile void * clock;
  mutable std::mutex mutex_;
  
  void HappensAfter() {
//    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::unique_lock<std::mutex> lock(mutex_);
    if (clock)
      TsanAfterClock(&clock);
  }
  
  void HappensBefore() {
    std::unique_lock<std::mutex> lock(mutex_);
    TsanBeforeClock(&clock);
  }
  
  SyncClock () : clock(nullptr), mutex_() {}
  SyncClock (int) : clock(nullptr), mutex_() {}
  ~SyncClock() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (clock)
      TsanDestroyClock(&clock);
  }
};

/// Data structure to store additional information for parallel regions.
struct ParallelData {
  /// Its address is used for relationships between the parallel region and
  /// its implicit tasks.
  ompt_tsan_clockid Parallel;

  /// Two addresses for relationships with barriers.
  ompt_tsan_clockid Barrier[2];

  void *GetParallelPtr() {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    Parallel=1;
#endif
    return &Parallel;
  }

  void *GetBarrierPtr(unsigned Index) {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    Barrier[Index]=1;
#endif
    return &Barrier[Index];
  }
  
  ParallelData() : Parallel(0) {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    Barrier[0] = Barrier[1] = 0;
#endif
  }

  ~ParallelData() {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    if (Parallel)
      TsanDeleteClock(&Parallel);
    if (Barrier[0])
      TsanDeleteClock(Barrier);
    if (Barrier[1])
      TsanDeleteClock(Barrier+1);
#elif defined(OMPT_TSAN_CLEANUP)
    TsanDeleteClock(&Parallel);
    TsanDeleteClock(Barrier);
    TsanDeleteClock(Barrier+1);
#endif
  }
};

static inline ParallelData *ToParallelData(ompt_data_t* parallel_data) {
  return reinterpret_cast<ParallelData*>(parallel_data->ptr);
}


/// Data structure to support stacking of taskgroups and allow synchronization.
struct Taskgroup {
  /// Its address is used for relationships of the taskgroup's task set.
  ompt_tsan_clockid Ptr;

  /// Reference to the parent taskgroup.
  Taskgroup* Parent;

  Taskgroup(Taskgroup* Parent) : Ptr(0), Parent(Parent) { }

  void *GetPtr() {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    Ptr=1;
#endif
    return &Ptr;
  }
  ~Taskgroup() {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    if (Ptr)
      TsanDeleteClock(&Ptr);
#elif defined(OMPT_TSAN_CLEANUP)
    TsanDeleteClock(&Ptr);
#endif
  }
};

/// Data structure to store additional information for tasks.
struct TaskData {
  /// Its address is used for relationships of this task.
  ompt_tsan_clockid Task;

  /// Child tasks use its address to declare a relationship to a taskwait in
  /// this task.
  ompt_tsan_clockid Taskwait;

  /// Whether this task is currently executing a barrier.
  bool InBarrier;

  /// Index of which barrier to use next.
  char BarrierIndex;

  /// Count how often this structure has been put into child tasks + 1.
  std::atomic_int RefCount;

  /// Reference to the parent that created this task.
  TaskData* Parent;

  /// Reference to the task that scheduled this task.
  TaskData* ScheduleParent;

  /// Reference to the task that scheduled this task.
  TaskData* ImplicitTask;

  /// Reference to the team of this task.
  ParallelData* Team;

  /// Reference to the current taskgroup that this task either belongs to or
  /// that it just created.
  Taskgroup* Taskgroup;

  /// Dependency information for this task.
  ompt_task_dependence_t* Dependencies;

  /// Number of dependency entries.
  unsigned DependencyCount;
  
#ifdef OMPT_TSAN_CLEANUP
  /// Set of all dependencies used by child tasks, used to cleanup the clocks
  std::set<void *> DependencySet;
#endif

  TaskData(TaskData* Parent) : Task(0), Taskwait(0), InBarrier(false), BarrierIndex(0), 
    RefCount(1), Parent(Parent), ScheduleParent(nullptr), ImplicitTask(nullptr), Team(Parent->Team), Taskgroup(nullptr), DependencyCount(0) {
    if (Parent != nullptr) {
      Parent->RefCount++;
      // Copy over pointer to taskgroup. This task may set up its own stack
      // but for now belongs to its parent's taskgroup.
      Taskgroup = Parent->Taskgroup;
    }
  }

  TaskData(ParallelData* Team = nullptr) : Task(0), Taskwait(0), InBarrier(false), BarrierIndex(0), 
    RefCount(1), Parent(nullptr), ScheduleParent(nullptr), ImplicitTask(this), Team(Team), Taskgroup(nullptr), DependencyCount(0) {}

  void *GetTaskPtr() {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    Task=1;
#endif
    return &Task;
  }

  void *GetTaskwaitPtr() {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    Taskwait=1;
#endif
    return &Taskwait;
  }
  
  ~TaskData() {
#ifdef OMPT_TSAN_CLEANUP_ATOMIC
    if (Task)
      TsanDeleteClock(&Task);
    if (Taskwait)
      TsanDeleteClock(&Taskwait);
#elif defined(OMPT_TSAN_CLEANUP)
    TsanDeleteClock(&Task);
    TsanDeleteClock(&Taskwait);
#endif
#ifdef OMPT_TSAN_CLEANUP
    for (auto i : DependencySet) {
      TsanDeleteClock(i);
      TsanDeleteClock(ToInAddr(i));
    }
#endif
    if (DependencyCount > 0) {
      delete[] Dependencies;
    }
  }
};

static inline TaskData *ToTaskData(ompt_data_t *task_data) {
  return reinterpret_cast<TaskData*>(task_data->ptr);
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
  thread_data->value = my_next_id();
}
            
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
}

static void
ompt_tsan_parallel_end(
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  ompt_invoker_t invoker,
  const void *codeptr_ra)
{
  ParallelData* Data = ToParallelData(parallel_data);
//  TsanHappensAfter(Data->GetBarrierPtr(0));
//  TsanHappensAfter(Data->GetBarrierPtr(1));

  delete Data;
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
        break;
     case ompt_scope_end:
        TaskData* Data = ToTaskData(task_data);
        assert(Data->RefCount == 1 && "All tasks should have finished at the implicit barrier!");
        delete Data;
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
            break;
          }
        case ompt_sync_region_taskwait:
            
            break;
        case ompt_sync_region_taskgroup:
            Data->Taskgroup = new Taskgroup(Data->Taskgroup);
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
            break;
          }
        case ompt_sync_region_taskwait:
            TsanHappensAfter(Data->GetTaskwaitPtr());
            break;
        case ompt_sync_region_taskgroup:
          {
            assert(Data->Taskgroup != nullptr && "Should have at least one taskgroup!");

            TsanHappensAfter(Data->Taskgroup->GetPtr());

            // Delete this allocated taskgroup, all descendent task are finished by now.
            Taskgroup* Parent = Data->Taskgroup->Parent;
            delete Data->Taskgroup;
            Data->Taskgroup = Parent;
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
    ompt_task_type_t type,
    int has_dependences,
    const void *codeptr_ra)               /* pointer to outlined function */
{
  TaskData* Data;
  switch(type)
  {
    case ompt_task_explicit:
    case ompt_task_target:
    {
      Data = new TaskData(ToTaskData(parent_task_data));
      new_task_data->ptr = Data;

      // Use the newly created address. We cannot use a single address from the
      // parent because that would declare wrong relationships with other
      // sibling tasks that may be created before this task is started!
      TsanHappensBefore(Data->GetTaskPtr());
      break;
    }
    case ompt_task_initial:
    {
      ompt_data_t* parallel_data;
      ompt_get_parallel_info(0, &parallel_data, NULL);
      ParallelData* PData = new ParallelData;
      parallel_data->ptr = PData;

      Data = new TaskData(PData);
      new_task_data->ptr = Data;

      break;
    }
    default:
      break;
  }
}

static void
ompt_tsan_task_schedule(
    ompt_data_t *first_task_data,
    ompt_task_status_t prior_task_status,
    ompt_data_t *second_task_data)
{
  TaskData* ToTask = ToTaskData(second_task_data);
  if (prior_task_status != ompt_task_complete) // start execution of a task
  {  
    if (ToTask != nullptr) {
      // 1. Task will begin execution after it has been created.
      // 2. Task will resume after it has been switched away.
      TsanHappensAfter(ToTask->GetTaskPtr());

      for (unsigned i = 0; i < ToTask->DependencyCount; i++) {
          ompt_task_dependence_t* Dependency = &ToTask->Dependencies[i];

          TsanHappensAfter(Dependency->variable_addr);
          // in and inout dependencies are also blocked by prior in dependencies!
          if (Dependency->dependence_flags & ompt_task_dependence_type_out) {
              TsanHappensAfter(ToInAddr(Dependency->variable_addr));
          }
      }
    }


    TaskData* FromTask = ToTaskData(first_task_data);
    assert(FromTask != nullptr);
    assert(FromTask != ToTask);
    assert(FromTask->ImplicitTask != nullptr);
    ToTask->ScheduleParent = FromTask;
    ToTask->ImplicitTask = FromTask->ImplicitTask;
    // Task may be resumed at a later point in time.
    TsanHappensBefore(FromTask->GetTaskPtr());

    if (FromTask->InBarrier) {
      // We want to ignore writes in the runtime code during barriers,
      // but not when executing tasks with user code!
      TsanIgnoreWritesEnd();
    }
  } else { // task finished
    TaskData* Data = ToTaskData(first_task_data);
    // Task ends after it has been switched away.
    TsanHappensAfter(Data->GetTaskPtr());

    // Task will finish before a barrier in the surrounding parallel region ...
    ParallelData* PData = Data->Team;
    TsanHappensBefore(PData->GetBarrierPtr(Data->ImplicitTask->BarrierIndex));

    // ... and before an eventual taskwait by the parent thread.
    TsanHappensBefore(Data->Parent->GetTaskwaitPtr());

    if (Data->Taskgroup != nullptr) {
        // This task is part of a taskgroup, so it will finish before the
        // corresponding taskgroup_end.
        TsanHappensBefore(Data->Taskgroup->GetPtr());
    }
    for (unsigned i = 0; i < Data->DependencyCount; i++) {
        ompt_task_dependence_t* Dependency = &Data->Dependencies[i];

        // in dependencies block following inout and out dependencies!
        TsanHappensBefore(ToInAddr(Dependency->variable_addr));
        if (Dependency->dependence_flags & ompt_task_dependence_type_out) {
            TsanHappensBefore(Dependency->variable_addr);
        }
    }
    while (Data != nullptr && --Data->RefCount == 0) {
        TaskData* Parent = Data->Parent;
        delete Data;
        Data = Parent;
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
  if (ndeps > 0) {
    // Copy the data to use it in task_switch and task_end.
    TaskData* Data = ToTaskData(task_data);
    Data->Dependencies = new ompt_task_dependence_t[ndeps];
    std::memcpy(Data->Dependencies, deps, sizeof(ompt_task_dependence_t) * ndeps);
    Data->DependencyCount = ndeps;

#ifdef OMPT_TSAN_CLEANUP
    for (unsigned i = 0; i < Data->DependencyCount; i++) {
      Data->Parent->DependencySet.insert(Data->Dependencies[i].variable_addr);
    }
#endif
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
  TsanHappensBefore(ToWaitPtr(wait_id));

  {
    LocksMutex.lock();
    std::mutex& Lock = Locks[wait_id];
    LocksMutex.unlock();

    Lock.unlock();
  }
}

#define SET_CALLBACK_T(event, type) \
  ompt_callback_##type##_t tsan_##event = &ompt_tsan_##event; \
  ompt_set_callback(ompt_callback_##event, (ompt_callback_t) tsan_##event)

#define SET_CALLBACK(event) SET_CALLBACK_T(event, event)


static int ompt_tsan_initialize(
  ompt_function_lookup_t lookup,
  ompt_fns_t* fns
  ) {
  ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
  if (ompt_set_callback == NULL) {
    std::cerr << "Could not set callback, exiting..." << std::endl;
    std::exit(1);
  }
  ompt_get_parallel_info = (ompt_get_parallel_info_t) lookup("ompt_get_parallel_info");
  ompt_get_thread_data = (ompt_get_thread_data_t) lookup("ompt_get_thread_data");
  
  if (ompt_get_parallel_info == NULL) {
    fprintf(stderr, "Could not get inquiry function 'ompt_get_parallel_info', exiting...\n");
    exit(1);
  }

  SET_CALLBACK(thread_begin);
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


static void ompt_tsan_finalize(ompt_fns_t* fns)
{
//   printf("%d: ompt_event_runtime_shutdown\n", omp_get_thread_num());
}


ompt_fns_t* ompt_start_tool(
  unsigned int omp_version,
  const char *runtime_version)
{
  static ompt_fns_t ompt_fns = {&ompt_tsan_initialize,&ompt_tsan_finalize};
  return &ompt_fns;
}

