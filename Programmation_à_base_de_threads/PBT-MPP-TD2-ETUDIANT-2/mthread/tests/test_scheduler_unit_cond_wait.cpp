#include <cassert>
#include <cstdio>
#include <mthread.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}
int NB_VP = 2;
int NB_THS = 2;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

#ifdef __cplusplus
extern "C" {
#endif
void mthread_abort();
#ifdef __cplusplus
}
#endif

static volatile int has_abort = 0;
void mthread_abort() { has_abort = 1; }

#define Check_has_abort()                                                      \
  {                                                                            \
    if (has_abort == 0) {                                                      \
      fprintf(stderr, "\tAbort caught\n");                                     \
      return 1;                                                                \
    } else {                                                                   \
      has_abort = 0;                                                           \
    }                                                                          \
  }                                                                            \
  (void)(0)

#define Check_val(val)                                                         \
  {                                                                            \
    assert(res == (val));                                                      \
    if ((res != (val)) || (has_abort != 0)) {                                  \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  {
    mthread_cond_t cond;
    mthread_mutex_t mut;
    mthread_mutex_init(nullptr, &mut);

    int res;
    res = mthread_cond_init(&cond);
    Check_val(0);
    res = mthread_cond_wait(&cond, &mut);
    Check_val(MTHREAD_COND_ERROR_NOT_LOCKED);
  }
  
  return 0;
}
