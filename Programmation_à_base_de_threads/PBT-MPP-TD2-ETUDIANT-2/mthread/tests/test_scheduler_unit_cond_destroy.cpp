#include <cassert>
#include <cstdio>
#include <mthread.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}
int NB_VP = 3;
int NB_THS = 3;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

#define Check_val(val)                                                         \
  {                                                                            \
    assert(res == (val));                                                      \
    if ((res != (val))) {                                                      \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  {
    mthread_cond_t cond;
    int res;
    res = mthread_cond_init(&cond);
    Check_val(0);
    res = mthread_cond_destroy(&cond);
    Check_val(0);
  }

  {
    mthread_cond_t cond;
    int res;
    res = mthread_cond_init(&cond);
    Check_val(0);
    res = mthread_cond_destroy(&cond);
    Check_val(0);
    res = mthread_cond_destroy(&cond);
    Check_val(MTHREAD_COND_ERROR_INIT);
  }

  {
    mthread_cond_t cond;
    int res;
    res = mthread_cond_destroy(&cond);
    Check_val(MTHREAD_COND_ERROR_INIT);
  }

  return 0;
}
