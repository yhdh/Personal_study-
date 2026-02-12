#include <mthread_common_helpers.h>

#define Check_val(val)                                                         \
  {                                                                            \
    assert(res == (val));                                                      \
    if (res != (val)) {                                                        \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

atomic_flag lock = ATOMIC_FLAG_INIT;

#ifdef __cplusplus
extern "C" {
#endif

int sched_yield(void) {
  mthread_internal_spin_unlock(&lock);
  return 0;
}

#ifdef __cplusplus
}
#endif
int main() {

  mthread_internal_spin_lock(&lock);

  mthread_internal_spin_lock(&lock);
  return 0;
}