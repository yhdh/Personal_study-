#ifndef MTHREAD_VP_INTERNAL_H

#include <mthread.h>
#include <mthread_thread.h>
#include <mthread_vp_allocator.h>
#include <vector>

template <typename Sched> struct mthread_vp_s {
  int id = -1;
  std::list<mthread_thread_t *, mthread_regular_allocator<mthread_thread_t *>>
      available_thread;
  Sched scheduler;
  mthread_thread_t *current_thread = nullptr;
  mthread_thread_t *next_thread = nullptr;
  mthread_thread_t *idle = nullptr;
  void *(*idle_func)(void *) = nullptr;
  atomic_flag *registered_spinlock = nullptr;
};

template <typename Sched, typename Lock> struct vp_list_s {
  std::list<struct mthread_vp_s<Sched> *,
            mthread_regular_allocator<struct mthread_vp_s<Sched> *>>
      vp_list;
  std::vector<struct mthread_vp_s<Sched> *,
              mthread_regular_allocator<struct mthread_vp_s<Sched> *>>
      vp_vector;
  Lock vp_thread;
};

template <typename Sched, typename Lock> struct vp_list_s<Sched, Lock> vp_list;

extern void mthread_block_self(mthread_mutex_t *lock);
extern void mthread_block_self(atomic_flag *flag);
extern void mthread_wake_thread(mthread_thread_t *thread);

#define MTHREAD_VP_INTERNAL_H
#endif
