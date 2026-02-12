#ifndef MTHREAD_VP_THREADS_H

#include <cassert>
#include <pthread.h>

///////////////////////////////////
// LOCKS
///////////////////////////////////

// Mutex
class [[maybe_unused]] mthread_threads_pthread_mutex {
public:
  void lock() { pthread_mutex_lock(&mutex); }
  void unlock() { pthread_mutex_unlock(&mutex); }

private:
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
};

// Spinlock
class mthread_threads_pthread_spinlock {
public:
  void lock() { pthread_spin_lock(&spinlock); }
  void unlock() { pthread_spin_unlock(&spinlock); }
  mthread_threads_pthread_spinlock() {
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
  }

private:
  pthread_spinlock_t spinlock{};
};

///////////////////////////////////
// BARRIERS
///////////////////////////////////

// Pthread_barrier
class mthread_threads_pthread_barrier {
public:
  void barrier_init(int i) { pthread_barrier_init(&barrier, nullptr, i); }
  void barrier_wait() { pthread_barrier_wait(&barrier); }

private:
  pthread_barrier_t barrier;
};

///////////////////////////////////
// THREADS SUPPORT
///////////////////////////////////
template <typename Lock, typename Barrier> class mthread_threads_pthread {
public:
  void lock() { mutex.lock(); }
  void unlock() { mutex.unlock(); }
  void barrier_init(int i) { barrier.barrier_init(i); }
  void barrier_wait() { barrier.barrier_wait(); }
  int thread_create(void *(*func)(void *), void *arg) {
    pthread_t th;
    int res = pthread_create(&th, nullptr, func, arg);
    assert(res == 0);
    return res;
  }

private:
  Lock mutex;
  Barrier barrier;
};

#define MTHREAD_VP_THREADS_H
#endif
