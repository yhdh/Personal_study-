#include <cassert>
#include <mthread.h>
#include <unistd.h>

mthread_mutex_t mutex;

int NB_VP = 6;
int NB_THS = 20;

#define Check_val(val)                                                         \
  {                                                                            \
    assert(res == (val));                                                      \
    if (res != (val)) {                                                        \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

#define Check_val_ptr(val)                                                     \
  {                                                                            \
    assert(res == (val));                                                      \
    if (res != (val)) {                                                        \
      return NULL;                                                             \
    }                                                                          \
  }                                                                            \
  (void)(0)

void *thread_test([[maybe_unused]] void *arg) {
  int res;
  mthread_log("MAIN", "Hello world!\n");
  res = mthread_mutex_lock(&mutex);
  Check_val_ptr(0);
  for (int i = 0; i < 2 * NB_THS; i++) {
    mthread_yield();
  }
  res = mthread_mutex_unlock(&mutex);
  Check_val_ptr(0);
  mthread_log("MAIN", "Hello world! END\n");
  return nullptr;
}

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  int res;
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  res = mthread_mutex_init(nullptr, &mutex);
  Check_val(0);
  res = mthread_mutex_lock(&mutex);
  Check_val(0);

  struct mthread_thread_s *ths[NB_THS];

  mthread_log("MAIN", "In main thread\n");
  for (int i = 0; i < NB_THS; i++) {
    mthread_attr_t default_attr;
    default_attr.vp = i % NB_VP;
    ths[i] = mthread_create_thread(&default_attr, thread_test, nullptr);
  }

  for (int i = 0; i < 2 * NB_THS; i++) {
    mthread_yield();
  }

  res = mthread_mutex_unlock(&mutex);
  Check_val(0);
  for (int i = 0; i < NB_THS; i++) {
    void *resptr = nullptr;
    mthread_join(ths[i], &resptr);
    assert(resptr == nullptr);
  }

  mthread_log("MAIN", "In main thread EXIT\n");
  print_available_threads();

  return 0;
}
