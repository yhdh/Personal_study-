#include <cassert>
#include <cstdlib>
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

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  struct mthread_thread_s *ths[NB_THS];
  void *stacks[NB_THS];

  mthread_log("MAIN", "In main thread\n");
  for (int i = 0; i < NB_THS; i++) {
    mthread_attr_t default_attr;
    default_attr.vp = i % NB_VP;
    default_attr.stack_size = MTHREAD_DEFAULT_STACK_SIZE * 10;
    default_attr.stack = malloc(default_attr.stack_size);
    stacks[i] = default_attr.stack;
    ths[i] = mthread_create_thread(&default_attr, thread_test, nullptr);
  }

  for (int i = 0; i < NB_THS; i++) {
    void *res = nullptr;
    mthread_join(ths[i], &res);
    assert(res == nullptr);
    free(stacks[i]);
  }

  mthread_log("MAIN", "In main thread EXIT\n");
  print_available_threads();

  return 0;
}
