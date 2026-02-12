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
    mthread_sem_getvalue(&sem, &res);
    Check_val(1);
  }

  {
    mthread_sem_t sem;
    int res;
    int val;
    res = mthread_sem_getvalue(&sem, &val);
    Check_val(MTHREAD_SEM_ERROR_INIT);
  }

  {
    int res;
    int val;
    res = mthread_sem_getvalue(nullptr, &val);
    Check_val(MTHREAD_SEM_ERROR_NULL);
  }

  {
    int res;
    mthread_sem_t sem;
    res = mthread_sem_init(&sem, 0);
    Check_val(0);
    res = mthread_sem_getvalue(&sem, nullptr);
    Check_val(MTHREAD_SEM_ERROR_NULL);
  }
  

  return 0;
}
