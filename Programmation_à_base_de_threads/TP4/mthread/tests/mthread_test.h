#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <mthread.h>
#ifndef MTHREAD_TEST_H

int NB_VP = -1;
int NB_THS = -1;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

[[maybe_unused]] static void lib_mthread_init_test(int argc, char **argv) {
  if ((NB_VP == -1) && (NB_THS == -1)) {
    assert(argc == 3);
    NB_VP = (int)strtol(argv[1], nullptr, 10);
    NB_THS = (int)strtol(argv[2], nullptr, 10);
  }

  mthread_init_scheduler_test(NB_VP);
  mthread_log("MAIN", "Init with %d vps and %d threads\n", NB_VP, NB_THS);
}

#define MTHREAD_TEST_H
#endif
