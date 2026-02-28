#include <cassert>
#include <mthread.h>
#include <mthread_test.h>

void *thread_test([[maybe_unused]] void *arg) {
  mthread_yield();
  return nullptr;
}

#define NB_ROUND ((size_t)100000)

int main(int argc, char **argv) {
  NB_VP = 2;
  NB_THS = 50;

  lib_mthread_init_test(argc, argv);

  struct mthread_thread_s *ths[NB_THS];

  for (size_t j = 0; j < NB_ROUND; j++) {
    for (int i = 0; i < NB_THS; i++) {
      mthread_attr_t default_attr;
      default_attr.vp = NB_VP - 1;
      ths[i] = mthread_create_thread(&default_attr, thread_test, nullptr);
      mthread_yield();
    }

    for (int i = 0; i < NB_THS; i++) {
      void *res = nullptr;
      mthread_join(ths[i], &res);
      assert(res == nullptr);
    }
  }

  return 0;
}
