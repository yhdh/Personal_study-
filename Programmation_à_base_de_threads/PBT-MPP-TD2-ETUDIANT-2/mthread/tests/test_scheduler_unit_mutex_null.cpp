#include <cassert>
#include <mthread.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}
int NB_VP = 1;
int NB_THS = 1;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

#define Check_val(val)                                                         \
  {                                                                            \
    assert(res == (val));                                                      \
    if (res != (val)) {                                                        \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  {
    int res;
    res = mthread_mutex_lock(nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
  }

  {
    int res;
    res = mthread_mutex_unlock(nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
  }

  {
    int res;
    res = mthread_mutex_trylock(nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
  }

  {
    int res;
    res = mthread_mutex_init(nullptr, nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
  }

  return 0;
}
