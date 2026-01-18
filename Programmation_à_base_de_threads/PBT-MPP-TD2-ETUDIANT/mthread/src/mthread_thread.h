#ifndef MTHREAD_THREAD_H

#include <cstdio>
#include <deque>
#include <list>
#include <memory>
#include <mthread.h>
#include <ucontext.h>

typedef enum {
  mthread_running,
  mthread_blocked,
  mthread_blocked_ready,
  mthread_zombie,
  mthread_zombie_joinable,
  mthread_zombie_joined,
  mthread_not_initialized
} mthread_status;

typedef struct mthread_thread_s {
  mthread_status status = mthread_not_initialized;
  void *(*func)(void *) = nullptr;
  void *arg = nullptr;
  ucontext_t uc{};
  void *res = nullptr;
  mthread_attr_t attr;
} mthread_thread_t;

void print_thread_status(mthread_thread_t *th, mthread_thread_t *idle);

#define MTHREAD_THREAD_H
#endif
