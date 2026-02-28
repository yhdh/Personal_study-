#ifndef MTHREAD_SCHEDULER_FIFO_CLASS_H

#include <cassert>
#include <mthread.h>
#include <mthread_scheduler_helpers.h>
#include <mthread_scheduler_loadbalancer.h>
#include <mthread_thread.h>
#include <mthread_vp_internal.h>

template <typename SchedLock, typename Lock, typename LoadBalancer>
class fifo_class_scheduler_t {
public:
  void find_next_thread(
      struct mthread_vp_s<fifo_class_scheduler_t<SchedLock, Lock, LoadBalancer>>
          *vp) {
    assert(vp->next_thread == NULL);

    // Get incoming threads
    vp->scheduler.lock.lock();
    if (vp->scheduler.incoming_thread.size() > 0) {
      vp->scheduler.ready_thread.splice(vp->scheduler.ready_thread.begin(),
                                        vp->scheduler.incoming_thread);
    }
    vp->scheduler.lock.unlock();

    // Search in ready queue
    while (vp->next_thread == NULL) {
      if (vp->scheduler.ready_thread.size() > 0) {
        vp->next_thread = vp->scheduler.ready_thread.front();
      }
      if (vp->next_thread == NULL) {
        break;
      } else if (vp->next_thread->status != mthread_running) {
        // Move to the other list
        if (vp->next_thread->status == mthread_zombie) {
          // remove element: no more accessible in this VP
          vp->scheduler.ready_thread.pop_front();
          vp->next_thread->status = mthread_zombie_joinable;
        } else if (vp->next_thread->status == mthread_blocked) {
          mthread_log("SCHED", "Remove blocked thread %p\n", vp->next_thread);
          // remove element: no more accessible in this VP
          vp->scheduler.ready_thread.pop_front();
          vp->next_thread->status = mthread_blocked_ready;
        } else {
          not_reachable();
        }
        vp->next_thread = NULL;
      }
    }
  }

  void reschedule_current(
      struct mthread_vp_s<fifo_class_scheduler_t<SchedLock, Lock, LoadBalancer>>
          *vp) {
    assert(vp->current_thread != NULL);
    // Move to the end of ready list
    if (vp->scheduler.ready_thread.size() > 1) {
      mthread_move_head_to_tail(vp->scheduler.ready_thread,
                                vp->scheduler.ready_thread);
    }
  }

  // Have to insert thread at the beginning of the list because current thread
  // running is the first element of the list
  void insert_new_thread(
      struct mthread_vp_s<fifo_class_scheduler_t<SchedLock, Lock, LoadBalancer>>
          *vp,
      mthread_thread_t *thread) {
    assert(thread->attr.vp != -1);
    if (thread->attr.vp == vp->id) {
      vp->scheduler.ready_thread.push_front(thread);
    } else {
      struct mthread_vp_s<
          fifo_class_scheduler_t<SchedLock, Lock, LoadBalancer>> *remote_vp =
          vp_list<fifo_class_scheduler_t<SchedLock, Lock, LoadBalancer>, Lock>.vp_vector
              [thread->attr.vp];
      remote_vp->scheduler.lock.lock();
      remote_vp->scheduler.incoming_thread.push_front(thread);
      remote_vp->scheduler.lock.unlock();
    }
  }

  int loadbalancer(
      [[maybe_unused]] struct mthread_vp_s<
          fifo_class_scheduler_t<SchedLock, Lock, LoadBalancer>> *vp,
      [[maybe_unused]] mthread_thread_t *thread) {
    return balancer.get_vp_id();
  }

  // NOLINTBEGIN
  void print_scheduler_status(
      struct mthread_vp_s<fifo_class_scheduler_t<SchedLock, Lock, LoadBalancer>>
          *vp) {
    fprintf(stderr, "\n\tPrint threads vp %d ready:\n", vp->id);
    mthread_print_list(vp->scheduler.ready_thread, vp->idle);
    vp->scheduler.lock.lock();
    fprintf(stderr, "\n\tPrint threads vp %d incoming:\n", vp->id);
    mthread_print_list(vp->scheduler.incoming_thread, vp->idle);
    vp->scheduler.lock.unlock();
  }
  // NOLINTEND

private:
  std::list<mthread_thread_t *, mthread_buffered_allocator<mthread_thread_t *>>
      ready_thread;
  std::list<mthread_thread_t *, mthread_buffered_allocator<mthread_thread_t *>>
      incoming_thread;
  SchedLock lock;
  LoadBalancer balancer;
};

// END Define new FIFO_CLASS scheduler
#define MTHREAD_SCHEDULER_FIFO_CLASS_H
#endif
