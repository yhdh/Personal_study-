#include <cassert>
#include <cstdio>
#include <mthread.h>
#include <mthread_vp_internal.h>

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

  {
    mthread_mutex_t lock;
    mthread_block_self(&lock);
    Check_has_abort();
  }

  {
    mthread_thread_t thread;
    mthread_wake_thread(&thread);
    Check_has_abort();
  }

  return 0;
}
