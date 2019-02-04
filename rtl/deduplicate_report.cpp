
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "sanitizer_common/sanitizer_atomic.h"
#include "tsan_interface.h"
#include "tsan_report.h"

#include "tsanToProtobuf.hpp"



// void*
// malloc_or_die(size_t bytes)
// {
//   void* temp = malloc(bytes);
//   if (temp == NULL) {
//     fprintf(stderr, "Unable to malloc for deduplication\n");
//     exit(-1);
//   }
//   return temp;
// }




// struct RaceSource {
//   int line;
//   int column;
//   int write_p;
//   char *filename;
//   int kept;
// };

// void
// init_RaceSource(struct RaceSource *r,
// 		int line, int column, int write_p, char *filename)
// {
//   assert(r != NULL);
//   assert(filename != NULL);

//   r->line = line;
//   r->column = column;
//   r->write_p = write_p;
//   r->filename = filename;
//   r->kept = 0;
// }

// void
// keep_RaceSource(struct RaceSource *r)
// {
//   assert(r != NULL);
//   assert(r->filename != NULL);

//   if (r->kept == 1) { return; }

//   r->kept = 1;

//   int len = strlen(r->filename);
//   char *temp = (char*) malloc_or_die(sizeof(char)*(len+1));
//   strcpy(temp, r->filename);
//   r->filename = temp;
// }

// int
// compare_RaceSource(const struct RaceSource *ra, const struct RaceSource *rb)
// {
//   assert(ra != NULL);
//   assert(ra->filename != NULL);
//   assert(rb != NULL);
//   assert(rb->filename != NULL);

//   if (ra->line != rb->line) { return ra->line - rb->line; }
//   if (ra->column != rb->column) { return ra->column - rb->column; }
//   if (ra->write_p != rb->write_p) { return ra->write_p - rb->write_p; }

//   return strcmp(ra->filename, rb->filename);
// }

// void
// copy_RaceSource(struct RaceSource *rd,  struct RaceSource *rs)
// {
//   assert(rd != NULL);
//   assert(rs != NULL);
//   assert(rs->filename != NULL);

//   rd->line = rs->line;
//   rd->column = rs->column;
//   rd->write_p = rs->write_p;
//   rd->filename = rs->filename;
//   rd->kept = rs->kept;
// }

// void
// swap_RaceSource(struct RaceSource *ra,  struct RaceSource *rb)
// {
//   assert(ra != NULL);
//   assert(ra->filename != NULL);
//   assert(rb != NULL);
//   assert(rb->filename != NULL);

//   int temp_line = ra->line;
//   int temp_column = ra->column;
//   int temp_write_p = ra->write_p;
//   char *temp_filename = ra->filename;
//   int temp_kept = ra->kept;

//   copy_RaceSource(ra, rb);

//   rb->line = temp_line;
//   rb->column = temp_column;
//   rb->write_p = temp_write_p;
//   rb->filename = temp_filename;
//   rb->kept = temp_kept;
// }




// struct Race {
//   int count;
//   int max_count;
//   struct RaceSource *sources;
//   int kept;
// };

// void
// init_Race(struct Race *R, int max_count, struct RaceSource *sources)
// {
//   assert(R != NULL);
//   assert(sources != NULL);

//   R->count = 0;
//   R->max_count = max_count;
//   R->sources = sources;
//   R->kept = 0;
// }

// void
// keep_Race(struct Race *R)
// {
//   assert(R != NULL);
//   assert(R->sources != NULL);

//   if (R->kept == 1) { return; }

//   R->kept = 1;

//   struct RaceSource *temp = (struct RaceSource *)
//     malloc_or_die(sizeof(struct RaceSource) * R->count);

//   for (int i=0; i<R->count; i++) {
//     copy_RaceSource(&temp[i], &R->sources[i]);
//     keep_RaceSource(&temp[i]);
//   }

//   R->sources = temp;
// }

// int
// compare_Race(struct Race *Ra, struct Race *Rb)
// {
//   assert(Ra != NULL);
//   assert(Ra->sources != NULL);
//   assert(Rb != NULL);
//   assert(Rb->sources != NULL);

//   if (Ra->count != Rb->count) { return Ra->count - Rb->count; }

//   for (int i=0; i<Ra->count; i++) {
//     int cmp = compare_RaceSource(&Ra->sources[i], &Rb->sources[i]);
//     if (cmp != 0) { return cmp; }
//   }

//   return 0;
// }

// void
// copy_Race(struct Race *Rd, struct Race *Rs)
// {
//   assert(Rd != NULL);
//   assert(Rs != NULL);
//   assert(Rs->sources != NULL);

//   Rd->count = Rs->count;
//   Rd->max_count = Rs->max_count;
//   Rd->sources = Rs->sources;
//   Rd->kept = Rs->kept;
// }

// void
// swap_Race(struct Race *Ra, struct Race *Rb)
// {
//   assert(Ra != NULL);
//   assert(Ra->sources != NULL);
//   assert(Rb != NULL);
//   assert(Rb->sources != NULL);

//   int temp_count = Ra->count;
//   int temp_max_count = Ra->max_count;
//   struct RaceSource *temp_sources = Ra->sources;
//   int temp_kept = Ra->kept;

//   copy_Race(Ra, Rb);

//   Rb->count = temp_count;
//   Rb->max_count = temp_max_count;
//   Rb->sources = temp_sources;
//   Rb->kept = temp_kept;
// }

// void
// add_RaceSource(struct Race *R, struct RaceSource *r)
// {
//   assert(R != NULL);
//   assert(R->count + 1 <= max_count);
//   assert(r != NULL);

//   struct RaceSource temp;
//   init_RaceSource(&temp, r->line, r->column, r->write_p, r->filename);
//   int insert_index = R->count;
//   for (int i=0; i<R->count; i++) {
//     if (compare_RaceSource(&R->sources[i], &temp) < 0) {
//       continue;
//     }
//     insert_index = i;
//     break;
//   }

//   R->count += 1;
//   for (int i=insert_index; i<R->count; i++) {
//     swap_RaceSource(&temp, &R->sources[i]);
//   }
// }




// struct RaceSet {
//   int count;
//   int max_count;
//   struct Race *races;
// };

// struct RaceSet*
// malloc_RaceSet(int max_count)
// {
//   struct RaceSet *temp = (struct RaceSet *)
//     malloc_or_die(sizeof(struct RaceSet));

//   temp->races = (struct Race *) malloc_or_die(sizeof(struct Race)*max_count);
//   temp->count = 0;
//   temp->max_count = max_count;
//   return temp;
// }

// int
// add_Race(struct RaceSet *RS, struct Race *R)
// {
//   int insert_index = RS->count;
//   for (int i=0; i<RS->count; i++) {
//     int cmp = compare_Race(&RS->races[i], R);
//     if (cmp == 0) {
//       return 0;
//     }
//     if (cmp < 0) {
//       continue;
//     }
//     insert_index = i;
//     break;
//   }

//   if (RS->count + 1 >= RS->max_count) { return 1; }

//   RS->count += 1;
//   struct Race temp;
//   copy_Race(&temp, R);
//   keep_Race(&temp);
//   for (int i=insert_index; i<RS->count; i++) {
//     swap_Race(&temp, &RS->races[i]);
//   }

//   return 1;
// }




// static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
// static RaceSet *race_set = NULL;

// namespace __tsan {
// bool  OnReport(const ReportDesc *report, bool suppressed) {

//     if (suppressed) {
//       return true;
//     }

//     if (race_set == NULL) {
//       race_set = malloc_RaceSet(1024);
//     }

//     uptr racers = report->mops.Size();
//     struct RaceSource this_sources[racers];
//     struct Race this_race;
//     init_Race(&this_race, racers, this_sources);
//     struct RaceSource this_source;
//     for (uptr i=0; i<racers; i++) {
//       const ReportMop *mop = report->mops[i];
//       const ReportStack *stack = mop->stack;

//       if (stack == nullptr || stack->frames == nullptr) {
// 	Printf("unable to restore the stack, hence unable to deduplicate\n\n");
// 	return suppressed;
//       }

//       const SymbolizedStack *frames = stack->frames;
//       const AddressInfo info = frames->info;

//       init_RaceSource(&this_source, info.line, info.column, mop->write, info.file);
//       add_RaceSource(&this_race, &this_source);
//     }

//     int retval = suppressed;

//     pthread_mutex_lock(&m);
//     if (add_Race(race_set, &this_race) == 0) {
//       retval = true;
//     }
//     pthread_mutex_unlock(&m);

//     return retval;
//   }
// }


namespace __tsan {
  bool  OnReport(const ReportDesc *report, bool suppressed) {
    PBReportDesc* pb_report = tsToPbReportDesc(*report);
    
    size_t size = pb_report->ByteSizeLong();
    void *bytes = malloc(size); 
    pb_report->SerializeToArray(bytes, size);

    Printf("==================\n");
    write(2, (char*)bytes, size);
    Printf("\n==================\n");

    delete pb_report;
    free(bytes);

    return true;
  }
}
