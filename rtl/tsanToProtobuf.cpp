#include "tsan_interface.h"
#include "tsan_report.h"
#include "tsanToProtobuf.hpp"

#include <cassert>
#include <stdio.h>




PBReportType tsToPbReportType (const __tsan::ReportType& typ) {
  switch (typ) {
  case __tsan::ReportTypeRace:
    return PBReportTypeRace;
  case __tsan::ReportTypeVptrRace:
    return PBReportTypeVptrRace;
  case __tsan::ReportTypeUseAfterFree:
    return PBReportTypeUseAfterFree;
  case __tsan::ReportTypeVptrUseAfterFree:
    return PBReportTypeVptrUseAfterFree;
  case __tsan::ReportTypeExternalRace:
    return PBReportTypeExternalRace;
  case __tsan::ReportTypeThreadLeak:
    return PBReportTypeThreadLeak;
  case __tsan::ReportTypeMutexDestroyLocked:
    return PBReportTypeMutexDestroyLocked;
  case __tsan::ReportTypeMutexDoubleLock:
    return PBReportTypeMutexDoubleLock;
  case __tsan::ReportTypeMutexInvalidAccess:
    return PBReportTypeMutexInvalidAccess;
  case __tsan::ReportTypeMutexBadUnlock:
    return PBReportTypeMutexBadUnlock;
  case __tsan::ReportTypeMutexBadReadLock:
    return PBReportTypeMutexBadReadLock;
  case __tsan::ReportTypeMutexBadReadUnlock:
    return PBReportTypeMutexBadReadUnlock;
  case __tsan::ReportTypeSignalUnsafe:
    return PBReportTypeSignalUnsafe;
  case __tsan::ReportTypeErrnoInSignal:
    return PBReportTypeErrnoInSignal;
  case __tsan::ReportTypeDeadlock:
    return PBReportTypeDeadlock;
  }
}

PBModuleArch tsToPbModuleArch (const __tsan::ModuleArch& typ) {
  switch (typ) {
  case __asan::kModuleArchUnknown:
    return PBkModuleArchUnknown;
  case __asan::kModuleArchI386:
    return PBkModuleArchI386;
  case __asan::kModuleArchX86_64:
    return PBkModuleArchX86_64;
  case __asan::kModuleArchX86_64H:
    return PBkModuleArchX86_64H;
  case __asan::kModuleArchARMV6:
    return PBkModuleArchARMV6;
  case __asan::kModuleArchARMV7:
    return PBkModuleArchARMV7;
  case __asan::kModuleArchARMV7S:
    return PBkModuleArchARMV7S;
  case __asan::kModuleArchARMV7K:
    return PBkModuleArchARMV7K;
  case __asan::kModuleArchARM64:
    return PBkModuleArchARM64;
  }
}

PBReportLocationType tsToPbReportLocationType (const __tsan::ReportLocationType& typ) {
  switch (typ) {
  case __tsan::ReportLocationGlobal:
    return PBReportLocationGlobal;
  case __tsan::ReportLocationHeap:
    return PBReportLocationHeap;
  case __tsan::ReportLocationStack:
    return PBReportLocationStack;
  case __tsan::ReportLocationTLS:
    return PBReportLocationTLS;
  case __tsan::ReportLocationFD:
    return PBReportLocationFD;
  }
}

PBAddressInfo* tsToPbAddressInfo (const __tsan::AddressInfo& tsan_obj) {
  PBAddressInfo* pb_obj = new PBAddressInfo();
  pb_obj->set_address(tsan_obj.address);

  if (tsan_obj.module) {
    pb_obj->set_module(tsan_obj.module);
  }
  pb_obj->set_module_offset(tsan_obj.module_offset);
  pb_obj->set_module_arch(tsToPbModuleArch(tsan_obj.module_arch));

  if (tsan_obj.function) {
    pb_obj->set_function(tsan_obj.function);
  }
  pb_obj->set_function_offset(tsan_obj.function_offset);

  if (tsan_obj.file) {
    pb_obj->set_file(tsan_obj.file);
  }
  pb_obj->set_line(tsan_obj.line);
  pb_obj->set_column(tsan_obj.column);

  return pb_obj;
}

PBSymbolizedStack* tsToPbSymbolizedStack (const __tsan::SymbolizedStack& tsan_obj) {
  PBSymbolizedStack* pb_obj = new PBSymbolizedStack();

  pb_obj->set_allocated_info(tsToPbAddressInfo(tsan_obj.info));
  if (tsan_obj.next) {
    pb_obj->set_allocated_next(tsToPbSymbolizedStack(*(tsan_obj.next)));
  }
  return pb_obj;
}

PBReportStack* tsToPbReportStack (const __tsan::ReportStack& tsan_obj) {
  PBReportStack* pb_obj = new PBReportStack();

  if (tsan_obj.frames) {
    pb_obj->set_allocated_frames(tsToPbSymbolizedStack(*(tsan_obj.frames)));
  }
  pb_obj->set_suppressable(tsan_obj.suppressable);

  return pb_obj;
}

PBReportMopMutex* tsToPbReportMopMutex (const __tsan::ReportMopMutex& tsan_obj) {
  PBReportMopMutex* pb_obj = new PBReportMopMutex();

  pb_obj->set_id(tsan_obj.id);
  pb_obj->set_write(tsan_obj.write);

  return pb_obj;
}

PBReportMop* tsToPbReportMop (const __tsan::ReportMop& tsan_obj) {
  PBReportMop* pb_obj = new PBReportMop();

  pb_obj->set_tid(tsan_obj.tid);
  pb_obj->set_addr(tsan_obj.addr);
  pb_obj->set_size(tsan_obj.size);
  pb_obj->set_write(tsan_obj.write);
  pb_obj->set_atomic(tsan_obj.atomic);
  pb_obj->set_external_tag(tsan_obj.external_tag);

  for (unsigned long i = 0; i < tsan_obj.mset.Size(); i++) {
    pb_obj->mutable_mset()->AddAllocated(tsToPbReportMopMutex(tsan_obj.mset[i]));
  }

  if (tsan_obj.stack) {
    pb_obj->set_allocated_stack(tsToPbReportStack(*(tsan_obj.stack)));
  }

  return pb_obj;
}

PBDataInfo* tsToPbDataInfo (const __tsan::DataInfo& tsan_obj) {
  PBDataInfo* pb_obj = new PBDataInfo();

  if (tsan_obj.module) {
    pb_obj->set_module(tsan_obj.module);
  }
  pb_obj->set_module_offset(tsan_obj.module_offset);
  pb_obj->set_module_arch(tsToPbModuleArch(tsan_obj.module_arch));

  if (tsan_obj.file) {
    pb_obj->set_file(tsan_obj.file);
  }
  pb_obj->set_line(tsan_obj.line);
  if (tsan_obj.name) {
    pb_obj->set_name(tsan_obj.name);
  }
  pb_obj->set_start(tsan_obj.start);
  pb_obj->set_size(tsan_obj.size);

  return pb_obj;
}



PBReportLocation* tsToPbReportLocation (const __tsan::ReportLocation& tsan_obj) {
  PBReportLocation* pb_obj = new PBReportLocation();

  pb_obj->set_type(tsToPbReportLocationType(tsan_obj.type));
  pb_obj->set_allocated_global(tsToPbDataInfo(tsan_obj.global));
  pb_obj->set_heap_chunk_start(tsan_obj.heap_chunk_start);
  pb_obj->set_heap_chunk_size(tsan_obj.heap_chunk_size);
  pb_obj->set_external_tag(tsan_obj.external_tag);
  pb_obj->set_tid(tsan_obj.tid);
  pb_obj->set_fd(tsan_obj.fd);
  pb_obj->set_suppressable(tsan_obj.suppressable);

  if (tsan_obj.stack) {
    pb_obj->set_allocated_stack(tsToPbReportStack(*(tsan_obj.stack)));
  }

  return pb_obj;
}

PBReportMutex* tsToPbReportMutex (const __tsan::ReportMutex& tsan_obj) {
  PBReportMutex* pb_obj = new PBReportMutex();

  pb_obj->set_id(tsan_obj.id);
  pb_obj->set_addr(tsan_obj.addr);
  pb_obj->set_destroyed(tsan_obj.destroyed);

  if (tsan_obj.stack) {
    pb_obj->set_allocated_stack(tsToPbReportStack(*(tsan_obj.stack)));
  }
  return pb_obj;
}

PBReportThread* tsToPbReportThread (const __tsan::ReportThread& tsan_obj) {
  PBReportThread* pb_obj = new PBReportThread();

  pb_obj->set_id(tsan_obj.id);
  pb_obj->set_os_id(tsan_obj.os_id);
  pb_obj->set_running(tsan_obj.running);
  pb_obj->set_workerthread(tsan_obj.workerthread);

  if (tsan_obj.name) {
    pb_obj->set_name(tsan_obj.name);
  }
  pb_obj->set_parent_tid(tsan_obj.parent_tid);

  if (tsan_obj.stack) {
    pb_obj->set_allocated_stack(tsToPbReportStack(*(tsan_obj.stack)));
  }

  return pb_obj;
}

PBReportDesc* tsToPbReportDesc (const __tsan::ReportDesc& tsan_obj) {
  PBReportDesc* pb_obj = new PBReportDesc();

  pb_obj->set_typ(tsToPbReportType(tsan_obj.typ));
  pb_obj->set_tag(tsan_obj.tag);

  for (unsigned long i = 0; i < tsan_obj.stacks.Size(); i++) {
    pb_obj->mutable_stacks()->AddAllocated(tsToPbReportStack(*(tsan_obj.stacks[i])));
  }

  for (unsigned long i = 0; i < tsan_obj.mops.Size(); i++) {
    pb_obj->mutable_mops()->AddAllocated(tsToPbReportMop(*(tsan_obj.mops[i])));
  }

  for (unsigned long i = 0; i < tsan_obj.locs.Size(); i++) {
    pb_obj->mutable_locs()->AddAllocated(tsToPbReportLocation(*(tsan_obj.locs[i])));
  }

  for (unsigned long i = 0; i < tsan_obj.mutexes.Size(); i++) {
    pb_obj->mutable_mutexes()->AddAllocated(tsToPbReportMutex(*(tsan_obj.mutexes[i])));
  }

  for (unsigned long i = 0; i < tsan_obj.threads.Size(); i++) {
    pb_obj->mutable_threads()->AddAllocated(tsToPbReportThread(*(tsan_obj.threads[i])));
  }

  for (unsigned long i = 0; i < tsan_obj.unique_tids.Size(); i++) {
    pb_obj->add_unique_tids(tsan_obj.unique_tids[i]);
  }

  if (tsan_obj.sleep) {
    pb_obj->set_allocated_sleep(tsToPbReportStack(*(tsan_obj.sleep)));
  }
  pb_obj->set_count(tsan_obj.count);

  return pb_obj;
}
