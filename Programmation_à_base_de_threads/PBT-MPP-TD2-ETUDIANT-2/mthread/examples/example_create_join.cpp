#include <cassert>
#include <mthread.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int NB_VP = 4;
  int NB_THS = 10;

  // Use fifo class scheduler
  mthread_init_scheduler_fifo_class(NB_VP);

  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  mthread_t ths[NB_THS];

  mthread_log("MAIN", "In main thread\n");

  for (int i = 0; i < NB_THS; i++) {
    mthread_attr_t default_attr;
    default_attr.vp = i % NB_VP;
    ths[i] = mthread_create_thread(&default_attr, thread_test, nullptr);
  }

  for (int i = 0; i < NB_THS; i++) {
    void *res;
    mthread_join(ths[i], &res);
    assert(res == nullptr);
  }

  mthread_log("MAIN", "In main thread EXIT\n");
  print_available_threads();

  return 0;
}
