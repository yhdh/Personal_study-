#ifndef MTHREAD_SCHEDULER_HELPERS_H

#include <mthread_common_helpers.h>
#include <mthread_thread.h>

template <typename T> void mthread_move_head_to_tail(T &from, T &to) {
  auto it = from.begin();
  auto end = to.end();
  to.splice(end, from, it);
}

template <typename T> void mthread_print_list(T &l, mthread_thread_t *vp_idle) {
  for (mthread_thread_t *t : l) {
    print_thread_status(t, vp_idle);
  }
}

#define MTHREAD_SCHEDULER_HELPERS_H
#endif
