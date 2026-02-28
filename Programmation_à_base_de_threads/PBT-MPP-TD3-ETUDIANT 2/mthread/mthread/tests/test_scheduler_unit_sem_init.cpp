#include <cassert>
#include <cstdio>
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
    if (res != (val)) {                                                        \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  {
    mthread_sem_t sem;
    int res;
    res = mthread_sem_init(&sem, 1);
    Check_val(0);
  }

  {
    mthread_sem_t sem;
    int res;
    res = mthread_sem_init(&sem, -1);
    Check_val(MTHREAD_SEM_ERROR_INIT);
  }

  {
    mthread_sem_t sem;
    int res;
    res = mthread_sem_init(&sem, 1);
    Check_val(0);
    res = mthread_sem_init(&sem, 1);
    Check_val(MTHREAD_SEM_ERROR_INIT);
  }
  

  return 0;
}
