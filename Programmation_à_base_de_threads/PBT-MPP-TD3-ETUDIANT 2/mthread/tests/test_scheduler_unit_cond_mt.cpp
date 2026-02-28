#include <cassert>
#include <cstdio>
#include <mthread.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}
int NB_VP = 2;
int NB_THS = 5;

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

mthread_cond_t cond;
mthread_mutex_t mut;

void* run_wait(void* arg) {
  mthread_mutex_lock(&mut);
  mthread_log("TEST", "WAIT\n");
  mthread_cond_wait(&cond, &mut);
  mthread_mutex_unlock(&mut);

  return nullptr;
}

void* run_signal(void* arg) {
  sleep(2);
  mthread_mutex_lock(&mut);
  mthread_log("TEST", "SIGNAL\n");
  mthread_cond_signal(&cond);
  mthread_mutex_unlock(&mut);

  return nullptr;
}

void* run_broadcast(void* arg)
{
  sleep(2);
  mthread_mutex_lock(&mut);
  mthread_log("TEST", "BROADCAST\n");
  mthread_cond_broadcast(&cond);
  mthread_mutex_unlock(&mut);

  return nullptr;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  mthread_mutex_init(nullptr, &mut);
  mthread_cond_init(&cond);

  struct mthread_thread_s *tids[NB_THS];

  for (int i = 0; i < NB_THS - 2; i++) {
    tids[i] = mthread_create_thread(nullptr, run_wait, nullptr);
    mthread_yield();
  }
  
  tids[NB_THS-2] = mthread_create_thread(nullptr, run_signal, nullptr);
  mthread_yield();
  tids[NB_THS-1] = mthread_create_thread(nullptr, run_broadcast, nullptr);
  mthread_yield();

  for (int i = 0; i < NB_THS; i++) {
    void *res = nullptr;
    mthread_join(tids[i], &res);
    assert(res == nullptr);
  }

  return 0;
}
