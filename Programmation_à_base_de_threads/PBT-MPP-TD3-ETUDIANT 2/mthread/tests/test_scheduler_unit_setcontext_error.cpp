#include <mthread.h>
#include <ucontext.h>

#include <mthread_valid_abort.h>
void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}
int NB_VP = 2;
constexpr int NB_THS = 1;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  wait_abort();

  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif
int setcontext([[maybe_unused]] const ucontext_t *ucp) { return 1; }

#ifdef __cplusplus
}
#endif
