
#include <mthread.h>
#include <mthread_scheduler_fifo.h>
#include <mthread_thread.h>

#include <mthread_vp_internal.h>
#include <mthread_vp_threads.h>

#define THREAD_NUMBER 3

int main() {
  struct fifo_scheduler_s<mthread_threads_pthread_spinlock,
                          mthread_threads_pthread_spinlock,
                          mthread_dummy_loadbalancer>
      sched;
  struct mthread_vp_s<struct fifo_scheduler_s<mthread_threads_pthread_spinlock,
                                              mthread_threads_pthread_spinlock,
                                              mthread_dummy_loadbalancer>>
      vp;
  vp.id = 0;

  mthread_self();
  mthread_yield();

  mthread_thread_t threads[THREAD_NUMBER];
  for (auto &thread : threads) {
    thread.attr.vp = vp.id;
  }

  for (auto &thread : threads) {
    sched.insert_new_thread(&vp, &thread);
  }

  threads[0].status = mthread_running;
  threads[1].status = mthread_blocked;
  threads[2].status = mthread_zombie;
  sched.print_scheduler_status(&vp);

  for (int i = 0; i < THREAD_NUMBER; i++) {
    vp.next_thread = nullptr;
    sched.find_next_thread(&vp);
    if (vp.next_thread != nullptr) {
      vp.current_thread = vp.next_thread;
      sched.reschedule_current(&vp);
    }
  }

  assert(threads[0].status == mthread_running);
  assert(threads[1].status == mthread_blocked_ready);
  assert(threads[2].status == mthread_zombie_joinable);

  sched.print_scheduler_status(&vp);

  threads[THREAD_NUMBER - 1].status = (mthread_status)42;

  sched.print_scheduler_status(&vp);
}