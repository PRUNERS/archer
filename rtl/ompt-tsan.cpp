#include <cstdlib>
#include <cstring>
#include <iostream>

#include <atomic>
#include <cassert>
#include <mutex>
#include <unordered_map>

#define __SANITIZE_THREAD__
#include "llvm/Support/Compiler.h"

#include <ompt.h>

#if LLVM_VERSION >= 40
class ArcherFlags {
public:
	int flush_shadow;

	ArcherFlags(const char *env) : flush_shadow(0) {
		if(env) {
			int ret = sscanf(env, "flush_shadow=%d", &flush_shadow);
			if(!ret) {
				std::cerr << "Illegal values for ARCHER_OPTIONS variable, no option has been set." << std::endl;
			}
		}
	}
};

extern "C" int __attribute__((weak)) __swordomp__get_omp_status();
extern "C" void __tsan_flush_memory();
ArcherFlags *archer_flags;
#endif

/// Required OMPT inquiry functions.
ompt_get_parallel_data_t ompt_get_parallel_data;

/// Data structure to store additional information for parallel regions.
struct ParallelData {
	/// Its address is used for relationships between the parallel region and
	/// its implicit tasks.
	char Parallel;

	/// Two addresses for relationships with barriers.
	char Barrier[2];

	void *GetParallelPtr() {
		return &Parallel;
	}

	void *GetBarrierPtr(unsigned Index) {
		return &Barrier[Index];
	}
};

static inline ParallelData *ToParallelData(ompt_parallel_data_t parallel_data) {
	return reinterpret_cast<ParallelData*>(parallel_data.ptr);
}


/// Data structure to support stacking of taskgroups and allow synchronization.
struct Taskgroup {
	/// Its address is used for relationships of the taskgroup's task set.
	char Ptr;

	/// Reference to the parent taskgroup.
	Taskgroup* Parent;

	Taskgroup(Taskgroup* Parent) : Parent(Parent) { }

	void *GetPtr() {
		return &Ptr;
	}
};

/// Data structure to store additional information for tasks.
struct TaskData {
	/// Its address is used for relationships of this task.
	char Task;

	/// Child tasks use its address to declare a relationship to a taskwait in
	/// this task.
	char Taskwait;

	/// Whether this task is currently executing a barrier.
	bool InBarrier;

	/// Index of which barrier to use next.
	char BarrierIndex;

	/// Count how often this structure has been put into child tasks + 1.
	std::atomic_int RefCount;

	/// Reference to the parent that created this task.
	TaskData* Parent;

	/// Reference to the current taskgroup that this task either belongs to or
	/// that it just created.
	Taskgroup* Taskgroup;

	/// Dependency information for this task.
	ompt_task_dependence_t* Dependencies;

	/// Number of dependency entries.
	unsigned DependencyCount;

	TaskData(TaskData* Parent = nullptr) : InBarrier(false), BarrierIndex(0),
			RefCount(1), Parent(Parent), Taskgroup(nullptr), DependencyCount(0) {
		if (Parent != nullptr) {
			Parent->RefCount++;
			// Copy over pointer to taskgroup. This task may set up its own stack
			// but for now belongs to its parent's taskgroup.
			Taskgroup = Parent->Taskgroup;
		}
	}

	void *GetTaskPtr() {
		return &Task;
	}

	void *GetTaskwaitPtr() {
		return &Taskwait;
	}
};

static inline TaskData *ToTaskData(ompt_task_data_t task_data) {
	return reinterpret_cast<TaskData*>(task_data.ptr);
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


/// OMPT event callbacks for handling parallel regions.

static void ompt_tsan_parallel_begin(ompt_task_data_t parent_task_data,
		ompt_frame_t *parent_task_frame, ompt_parallel_data_t *parallel_data,
		uint32_t requested_team_size, void *parallel_function,
		ompt_invoker_t invoker) {

	ParallelData* Data = new ParallelData;

	parallel_data->ptr = Data;

	TsanHappensBefore(Data->GetParallelPtr());
}

static void ompt_tsan_implicit_task_begin(ompt_parallel_data_t parallel_data,
		ompt_task_data_t *task_data) {
	task_data->ptr = new TaskData;

	// Implicit task will begin after the parallel region has started.
	TsanHappensAfter(ToParallelData(parallel_data)->GetParallelPtr());
}

static void ompt_tsan_barrier_begin(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data) {
	TaskData* Data = ToTaskData(task_data);
	char BarrierIndex = Data->BarrierIndex;
	TsanHappensBefore(ToParallelData(parallel_data)->GetBarrierPtr(BarrierIndex));

	// We ignore writes inside the barrier. These would either occur during
	// 1. reductions performed by the runtime which are guaranteed to be race-free.
	// 2. execution of another task.
	// For the latter case we will re-enable tracking in task_switch.
	Data->InBarrier = true;
	TsanIgnoreWritesBegin();
}

static void ompt_tsan_barrier_end(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data) {
	TaskData* Data = ToTaskData(task_data);

	// We want to track writes after the barrier again.
	Data->InBarrier = false;
	TsanIgnoreWritesEnd();

	char BarrierIndex = Data->BarrierIndex;
	// Barrier will end after it has been entered by all threads.
	TsanHappensAfter(ToParallelData(parallel_data)->GetBarrierPtr(BarrierIndex));

	// It is not guaranteed that all threads have exited this barrier before
	// we enter the next one. So we will use a different address.
	// We are however guaranteed that this current barrier is finished
	// by the time we exit the next one. So we can then reuse the first address.
	Data->BarrierIndex = (BarrierIndex + 1) % 2;
}

static void ompt_tsan_implicit_task_end(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data) {
	TaskData* Data = ToTaskData(task_data);
	assert(Data->RefCount == 1 && "All tasks should have finished at the implicit barrier!");
	delete Data;
}

static void ompt_tsan_parallel_end(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data, ompt_invoker_t invoker) {
	ParallelData* Data = ToParallelData(parallel_data);

	delete Data;

#if LLVM_VERSION >= 40
	if(&__swordomp__get_omp_status) {
		if(__swordomp__get_omp_status() == 0 && archer_flags->flush_shadow)
			__tsan_flush_memory();
	}
#endif
}


/// OMPT event callbacks for handling tasks.

static void ompt_tsan_task_begin(ompt_task_data_t parent_task_data,
		ompt_frame_t *parent_task_frame, ompt_task_data_t *new_task_data,
		void *task_function) {
	TaskData* Data = new TaskData(ToTaskData(parent_task_data));
	new_task_data->ptr = Data;

	// Use the newly created address. We cannot use a single address from the
	// parent because that would declare wrong relationships with other
	// sibling tasks that may be created before this task is started!
	TsanHappensBefore(Data->GetTaskPtr());
}

static void ompt_tsan_task_switch(ompt_task_data_t first_task_data,
		ompt_task_data_t second_task_data) {
	TaskData* ToTask = ToTaskData(second_task_data);
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

		if (ToTask->InBarrier) {
			// We re-enter runtime code which currently performs a barrier.
			TsanIgnoreWritesBegin();
		}
	}

	TaskData* FromTask = ToTaskData(first_task_data);
	if (FromTask != nullptr) {
		// Task may be resumed at a later point in time.
		TsanHappensBefore(FromTask->GetTaskPtr());

		if (FromTask->InBarrier) {
			// We want to ignore writes in the runtime code during barriers,
			// but not when executing tasks with user code!
			TsanIgnoreWritesEnd();
		}
	}
}

static void ompt_tsan_task_end(ompt_task_data_t task_data) {
	TaskData* Data = ToTaskData(task_data);
	// Task ends after it has been switched away.
	TsanHappensAfter(Data->GetTaskPtr());

	// Find implicit task with no parent to get the BarrierIndex.
	assert(Data->Parent != nullptr && "Should have at least an implicit task as parent!");
	TaskData* ImplicitTask = Data->Parent;
	while (ImplicitTask->Parent != nullptr) {
		ImplicitTask = ImplicitTask->Parent;
	}

	// Task will finish before a barrier in the surrounding parallel region ...
	ParallelData* PData = ToParallelData(ompt_get_parallel_data(0));
	TsanHappensBefore(PData->GetBarrierPtr(ImplicitTask->BarrierIndex));

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
		if (Data->DependencyCount > 0) {
			delete[] Data->Dependencies;
		}
		delete Data;
		Data = Parent;
	}
}

static void ompt_tsan_taskwait_end(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data) {
	TsanHappensAfter(ToTaskData(task_data)->GetTaskwaitPtr());
}

static void ompt_tsan_taskgroup_begin(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data) {
	TaskData* Data = ToTaskData(task_data);
	Data->Taskgroup = new Taskgroup(Data->Taskgroup);
}

static void ompt_tsan_taskgroup_end(ompt_parallel_data_t parallel_data,
		ompt_task_data_t task_data) {
	TaskData* Data = ToTaskData(task_data);
	assert(Data->Taskgroup != nullptr && "Should have at least one taskgroup!");

	TsanHappensAfter(Data->Taskgroup->GetPtr());

	// Delete this allocated taskgroup, all descendent task are finished by now.
	Taskgroup* Parent = Data->Taskgroup->Parent;
	delete Data->Taskgroup;
	Data->Taskgroup = Parent;
}

static void ompt_tsan_task_dependences(ompt_task_data_t task_data,
		const ompt_task_dependence_t *deps, int ndeps) {
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
static void ompt_tsan_lock_acquired(ompt_wait_id_t wait_id) {
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

static void ompt_tsan_lock_release(ompt_wait_id_t wait_id) {
	TsanHappensBefore(ToWaitPtr(wait_id));

	{
		LocksMutex.lock();
		std::mutex& Lock = Locks[wait_id];
		LocksMutex.unlock();

		Lock.unlock();
	}
}


static void ompt_tsan_initialize(ompt_function_lookup_t lookup,
		const char *runtime_version, unsigned int ompt_version) {
	ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t) lookup("ompt_set_callback");
	if (ompt_set_callback == NULL) {
		std::cerr << "Could not set callback, exiting..." << std::endl;
		std::exit(1);
	}
	ompt_get_parallel_data = (ompt_get_parallel_data_t) lookup("ompt_get_parallel_data");
	if (ompt_get_parallel_data == NULL) {
		fprintf(stderr, "Could not get inquiry function 'ompt_get_parallel_data', exiting...\n");
		exit(1);
	}

#if LLVM_VERSION >= 40
	const char *options = getenv("ARCHER_OPTIONS");
	archer_flags = new ArcherFlags(options);
#endif

	// Register parallel callbacks.
	ompt_set_callback(ompt_event_parallel_begin, (ompt_callback_t) &ompt_tsan_parallel_begin);
	ompt_set_callback(ompt_event_implicit_task_begin, (ompt_callback_t) &ompt_tsan_implicit_task_begin);
	ompt_set_callback(ompt_event_barrier_begin, (ompt_callback_t) &ompt_tsan_barrier_begin);
	ompt_set_callback(ompt_event_barrier_end, (ompt_callback_t) &ompt_tsan_barrier_end);
	ompt_set_callback(ompt_event_implicit_task_end, (ompt_callback_t) &ompt_tsan_implicit_task_end);
	ompt_set_callback(ompt_event_parallel_end, (ompt_callback_t) &ompt_tsan_parallel_end);

	// Register task callbacks.
	// FIXME: will be renamed to ompt_event_task_create!
	ompt_set_callback(ompt_event_task_begin, (ompt_callback_t) &ompt_tsan_task_begin);
	// FIXME: will be renamed to ompt_event_task_schedule!
	ompt_set_callback(ompt_event_task_switch, (ompt_callback_t) &ompt_tsan_task_switch);
	// FIXME: will only be implicitly given by task_schedule and prior_completed!
	ompt_set_callback(ompt_event_task_end, (ompt_callback_t) &ompt_tsan_task_end);

	ompt_set_callback(ompt_event_taskwait_end, (ompt_callback_t) &ompt_tsan_taskwait_end);
	ompt_set_callback(ompt_event_taskgroup_begin, (ompt_callback_t) &ompt_tsan_taskgroup_begin);
	ompt_set_callback(ompt_event_taskgroup_end, (ompt_callback_t) &ompt_tsan_taskgroup_end);

	ompt_set_callback(ompt_event_task_dependences, (ompt_callback_t) &ompt_tsan_task_dependences);

	// Register one single callback for acquiring locks and another one for
	// releasing all kind of locks: We don't need to differentiate here.
	ompt_set_callback(ompt_event_acquired_lock, (ompt_callback_t) &ompt_tsan_lock_acquired);
	ompt_set_callback(ompt_event_acquired_nest_lock_first, (ompt_callback_t) &ompt_tsan_lock_acquired);
	ompt_set_callback(ompt_event_acquired_critical, (ompt_callback_t) &ompt_tsan_lock_acquired);
	ompt_set_callback(ompt_event_acquired_atomic, (ompt_callback_t) &ompt_tsan_lock_acquired);
	ompt_set_callback(ompt_event_acquired_ordered, (ompt_callback_t) &ompt_tsan_lock_acquired);
	ompt_set_callback(ompt_event_release_lock, (ompt_callback_t) &ompt_tsan_lock_release);
	ompt_set_callback(ompt_event_release_nest_lock_last, (ompt_callback_t) &ompt_tsan_lock_release);
	ompt_set_callback(ompt_event_release_critical, (ompt_callback_t) &ompt_tsan_lock_release);
	ompt_set_callback(ompt_event_release_atomic, (ompt_callback_t) &ompt_tsan_lock_release);
	ompt_set_callback(ompt_event_release_ordered, (ompt_callback_t) &ompt_tsan_lock_release);
}

ompt_initialize_t ompt_tool() {
	return &ompt_tsan_initialize;
}
