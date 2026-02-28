#include <cassert>
#include <cstdlib>
#include <mthread.h>
#include <ucontext.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}
int NB_VP = 1;
constexpr int NB_THS = 1;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  struct mthread_thread_s *ths[NB_THS];

  mthread_log("MAIN", "In main thread\n");
  for (auto &th : ths) {
    mthread_attr_t default_attr;
    th = mthread_create_thread(&default_attr, thread_test, nullptr);
    assert(th == nullptr);
  }

  for (auto &th : ths) {
    void *res = nullptr;
    mthread_join(th, &res);
    assert(res == nullptr);
  }

  mthread_log("MAIN", "In main thread EXIT\n");
  print_available_threads();

  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif
int getcontext([[maybe_unused]] ucontext_t *ucp) { return 1; }

void abort(void) { exit(0); }

#ifdef __cplusplus
}
#endif
