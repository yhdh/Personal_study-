
#include <mthread.h>
#include <mthread_thread.h>

#include <mthread_scheduler_empty.h>
#include <mthread_vp_internal.h>

int main() {
  empty_scheduler_t sched;
  mthread_thread_t th;
  struct mthread_vp_s<empty_scheduler_t> vp;
  int i;

  mthread_self();
  mthread_yield();

  for (i = mthread_running; i <= mthread_not_initialized; i++) {
    th.status = static_cast<mthread_status>(i);
    print_thread_status(&th, nullptr);
  }
  th.status = static_cast<mthread_status>(i);
  print_thread_status(&th, nullptr);

  sched.print_scheduler_status(&vp);
}