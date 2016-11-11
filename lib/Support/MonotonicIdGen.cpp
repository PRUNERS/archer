#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

using namespace std;
using namespace boost::interprocess;

extern "C" {
  unsigned long long get_next_id()
  {
    unsigned long long id = 0;

    try {
      managed_shared_memory managed_shm(create_only, "archer_shm", 1024);
      named_mutex named_mtx(create_only, "archer_mtx");
      scoped_lock<named_mutex> lock(named_mtx);
      unsigned long long *i = managed_shm.find_or_construct<unsigned long long>("UnsignedLongLong")(0);
      id = (*i);
      ++(*i);
      return id;
    } catch (...) {
      managed_shared_memory managed_shm(open_only, "archer_shm");
      named_mutex named_mtx(open_only, "archer_mtx");
      scoped_lock<named_mutex> lock(named_mtx);
      unsigned long long *i = managed_shm.find_or_construct<unsigned long long>("UnsignedLongLong")(0);
      id = (*i);
      ++(*i);
      return id;
    }
  }
}
