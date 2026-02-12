#include <cassert>
#include <mthread.h>
#include <mthread_test.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_log("MAIN", "Hello world!\n");
  mthread_yield();
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}

int main(int argc, char **argv) {
  lib_mthread_init_test(argc, argv);

  struct mthread_thread_s *ths[NB_THS];

  mthread_log("MAIN", "In main thread\n");
  for (int i = 0; i < NB_THS; i++) {
    mthread_attr_t default_attr;
    default_attr.vp = i % NB_VP;
    ths[i] = mthread_create_thread(&default_attr, thread_test, nullptr);
    mthread_yield();
  }

  for (int i = 0; i < NB_THS; i++) {
    void *res = nullptr;
    mthread_join(ths[i], &res);
    assert(res == nullptr);
  }

  mthread_log("MAIN", "In main thread second round\n");
  for (int i = 0; i < NB_THS; i++) {
    mthread_attr_t default_attr;
    default_attr.vp = i % NB_VP;
    ths[i] = mthread_create_thread(&default_attr, thread_test, nullptr);
  }

  for (int i = 0; i < NB_THS; i++) {
    void *res = nullptr;
    mthread_join(ths[i], &res);
    assert(res == nullptr);
  }
  mthread_log("MAIN", "In main thread EXIT\n");
  print_available_threads();

  return 0;
}
