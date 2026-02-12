#include <mthread.h>
#include <mthread_common_helpers.h>
#include <mthread_thread.h>
#include <mthread_vp_internal.h>
#include <stdexcept>

int mthread_cond_init(mthread_cond_t *cond) {
  throw std::logic_error("Not implemented");
}

int mthread_cond_signal(mthread_cond_t *cond) {
  throw std::logic_error("Not implemented");
}

int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mut) {
  throw std::logic_error("Not implemented");
}

int mthread_cond_broadcast(mthread_cond_t *cond) {
  throw std::logic_error("Not implemented");
}

int mthread_cond_destroy(mthread_cond_t *cond) {
  throw std::logic_error("Not implemented");
}
