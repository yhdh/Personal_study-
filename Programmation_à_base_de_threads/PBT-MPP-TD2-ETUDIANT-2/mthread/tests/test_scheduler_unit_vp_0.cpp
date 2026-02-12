#include <mthread.h>

int NB_VP = 0;
int NB_THS = 0;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);

  return 0;
}
