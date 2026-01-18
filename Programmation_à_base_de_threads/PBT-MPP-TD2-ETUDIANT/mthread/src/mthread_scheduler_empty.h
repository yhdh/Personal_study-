#ifndef MTHREAD_SCHEDULER_EMPTY_H

#include "mthread_common_helpers.h"
#include <cassert>
#include <mthread.h>
#include <mthread_scheduler_helpers.h>
#include <mthread_thread.h>
#include <mthread_vp_internal.h>

class empty_scheduler_t {
public:
  static void find_next_thread(
      [[maybe_unused]] struct mthread_vp_s<empty_scheduler_t> *vp) {
    not_implemented();
  }

  static void reschedule_current(
      [[maybe_unused]] struct mthread_vp_s<empty_scheduler_t> *vp) {
    not_implemented();
  }

  static void
  insert_new_thread([[maybe_unused]] struct mthread_vp_s<empty_scheduler_t> *vp,
                    [[maybe_unused]] mthread_thread_t *thread) {
    not_implemented();
  }

  // NOLINTBEGIN
  void print_scheduler_status(
      [[maybe_unused]] struct mthread_vp_s<empty_scheduler_t> *vp) {
    return;
  }
  // NOLINTEND
};

// END Define new FIFO_CLASS scheduler
#define MTHREAD_SCHEDULER_EMPTY_H
#endif
