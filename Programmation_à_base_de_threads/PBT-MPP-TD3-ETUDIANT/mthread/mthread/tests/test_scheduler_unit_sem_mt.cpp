#include <cassert>
#include <cstdio>
#include <mthread.h>
#include <unistd.h>

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
    if (res != (val)) {                                                        \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

mthread_sem_t sem;

void* sem_wait(void* arg)
{
  mthread_sem_wait(&sem);
  mthread_sem_wait(&sem);
  return nullptr;
}

void* sem_post(void* arg)
{
  mthread_sem_post(&sem);
  return nullptr;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  mthread_sem_init(&sem, 1);

  {
    int res;
  
    mthread_t t1 = mthread_create_thread(nullptr, sem_post, nullptr);
    mthread_t t2 = mthread_create_thread(nullptr, sem_wait, nullptr);
    //mthread_t t2 = mthread_create_thread(nullptr, sem_post, nullptr);

    void* ret = nullptr;
    mthread_join(t1, &ret);
    mthread_join(t2, &ret);

    mthread_sem_getvalue(&sem, &res);
    Check_val(0);
  }

  return 0;
}
