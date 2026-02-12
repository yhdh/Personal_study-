#include <cstdio>
#include <cstdlib>
#include <mthread_common_helpers.h>
#include <mthread_thread.h>

static const char *mthread_status_convert_to_string(mthread_thread_t *th) {
  switch (th->status) {
  case mthread_running:
    return "running";
  case mthread_blocked:
    return "blocked";
  case mthread_blocked_ready:
    return "blocked_ready";
  case mthread_zombie:
    return "zombie";
  case mthread_zombie_joinable:
    return "zombie_joinable";
  case mthread_zombie_joined:
    return "zombie_joined";
  case mthread_not_initialized:
    return "not_initialized";
  }
  return "unknown";
}

void print_thread_status(mthread_thread_t *th, mthread_thread_t *idle) {
  const char *status = mthread_status_convert_to_string(th);

  if (th == idle) {
    fprintf(stderr, "\t\tThread %p IDLE %s (%d) VPs %d\n", th, status,
            th->status, th->attr.vp);
  } else {
    fprintf(stderr, "\t\tThread %p STD  %s (%d) VPs %d\n", th, status,
            th->status, th->attr.vp);
  }
}

void mthread_abort() { abort(); }
