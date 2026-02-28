#include <mthread.h>
#include <mthread_thread.h>
#include <mthread_common_helpers.h>
#include <mthread_vp_internal.h>
#include <stdexcept>


int mthread_sem_init(mthread_sem_t *sem, int value) {
  throw std::logic_error("Not implemented");
}

int mthread_sem_wait(mthread_sem_t *sem) {
  throw std::logic_error("Not implemented");
}

int mthread_sem_post(mthread_sem_t *sem) {
  throw std::logic_error("Not implemented");
}

int mthread_sem_getvalue(mthread_sem_t *sem, int *val) {
  throw std::logic_error("Not implemented");
}

int mthread_sem_trylock(mthread_sem_t *sem) {
  throw std::logic_error("Not implemented");
}

