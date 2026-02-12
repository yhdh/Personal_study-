#ifndef MTHREAD_H

#ifdef __cplusplus
#include <atomic>
#include <cstdio>
using namespace std;
#else
#include <stdatomic.h>
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

constexpr size_t MTHREAD_DEFAULT_STACK_SIZE = (16384 * 2);

typedef struct mthread_attr_s {
  int vp = -1;
  void *stack = nullptr;
  size_t stack_size = MTHREAD_DEFAULT_STACK_SIZE;
  bool buffered_stack = false;
  bool user_stack = false;
} mthread_attr_t;

struct mthread_thread_s;
typedef struct mthread_thread_s *mthread_t;

typedef struct mthread_list_item_s {
  struct mthread_thread_s *thread;
  struct mthread_list_item_s *next;
} mthread_list_item_t;

typedef struct mthread_list_s {
  volatile mthread_list_item_t *head = nullptr;
  volatile mthread_list_item_t *tail = nullptr;
} mthread_list_t;

enum {
  MTHREAD_MUTEX_ERROR_INIT = 1,
  MTHREAD_MUTEX_ERROR_NULL = 2,
  MTHREAD_MUTEX_ERROR_OWNER = 3,
  MTHREAD_MUTEX_ERROR_NOT_LOCKED = 4,
  MTHREAD_MUTEX_ERROR_ALREADY_LOCKED = 5
};

typedef struct mthread_mutex_s {
  bool is_initialized = false;
  atomic_bool is_locked = false;
  struct mthread_thread_s *owner = nullptr;
  atomic_flag thread_list_spinlock = ATOMIC_FLAG_INIT;
  mthread_list_t thread_list;
} mthread_mutex_t;

typedef struct {
  int dummy;
} mthread_mutex_attr_t;

int mthread_mutex_lock(mthread_mutex_t *lock);
int mthread_mutex_unlock(mthread_mutex_t *lock);
int mthread_mutex_trylock(mthread_mutex_t *lock);
int mthread_mutex_init(mthread_mutex_attr_t *attr, mthread_mutex_t *lock);

typedef struct mthread_sem_s {

/* TO DO */

} mthread_sem_t;

enum {
  MTHREAD_SEM_ERROR_INIT = 1,
  MTHREAD_SEM_ERROR_ALREADY_LOCKED = 2,
  MTHREAD_SEM_ERROR_NULL = 3,
};

int mthread_sem_init(mthread_sem_t *sem, int val);
int mthread_sem_wait(mthread_sem_t *sem);
int mthread_sem_post(mthread_sem_t *sem);
int mthread_sem_trylock(mthread_sem_t *sem);
int mthread_sem_getvalue(mthread_sem_t *sem, int *val);

// CONDITIONS 

typedef struct mthread_cond_s {

/* TO DO */

} mthread_cond_t;

enum {
  MTHREAD_COND_ERROR_INIT = 1,
  MTHREAD_COND_ERROR_IN_USE = 2,
  MTHREAD_COND_ERROR_NULL = 3,
  MTHREAD_COND_ERROR_NOT_LOCKED = 4,
};

int mthread_cond_init(mthread_cond_t *cond);
int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mut);
int mthread_cond_signal(mthread_cond_t *cond);
int mthread_cond_broadcast(mthread_cond_t *cond);
int mthread_cond_destroy(mthread_cond_t *cond);

// NOLINTBEGIN
extern mthread_t mthread_self();
extern void mthread_yield();
extern mthread_t mthread_create_thread(mthread_attr_t *attr,
                                       void *(*func)(void *), void *arg);
extern int mthread_join(mthread_t thread, void **res);
extern void print_available_threads();
extern int (*mthread_log)(const char *part, const char *format, ...);
// NOLINTEND

[[maybe_unused]] extern void mthread_init_scheduler_fifo(int);
extern void mthread_init_scheduler_fifo_class(int);

#ifdef __cplusplus
}
#endif

#define MTHREAD_H
#endif
