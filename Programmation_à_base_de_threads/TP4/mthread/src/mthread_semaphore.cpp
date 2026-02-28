#include <mthread.h>
#include <mthread_thread.h>
#include <mthread_common_helpers.h>
#include <mthread_vp_internal.h>


int mthread_sem_init(mthread_sem_t *sem, int value) {
  if (sem == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (value < 0) {
    return MTHREAD_SEM_ERROR_INIT;
  }
  if (sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }
  sem->is_initialized = true;
  atomic_store(&(sem->value), value);
  atomic_flag_clear(&(sem->thread_list_spinlock));
  sem->thread_list.head = nullptr;
  sem->thread_list.tail = nullptr;
  return 0;
}

int mthread_sem_wait(mthread_sem_t *sem) {
  if (sem == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (!sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }

  mthread_internal_spin_lock(&(sem->thread_list_spinlock));

  // If value > 0, decrement and return
  if (atomic_load(&(sem->value)) > 0) {
    atomic_fetch_sub(&(sem->value), 1);
    mthread_internal_spin_unlock(&(sem->thread_list_spinlock));
    return 0;
  }

  // Value is 0, block the calling thread
  mthread_thread_t *current_thread = mthread_self();
  mthread_list_item_t item;
  item.thread = current_thread;
  insert_tail_in_list(&item, &sem->thread_list);

  // Block and release the spinlock
  mthread_block_self(&(sem->thread_list_spinlock));

  return 0;
}

int mthread_sem_post(mthread_sem_t *sem) {
  if (sem == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (!sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }

  mthread_internal_spin_lock(&(sem->thread_list_spinlock));

  // If there are waiting threads, wake one up
  if (sem->thread_list.head != nullptr) {
    mthread_list_item_t *item = remove_head_in_list(&(sem->thread_list));
    mthread_internal_spin_unlock(&(sem->thread_list_spinlock));
    mthread_wake_thread(item->thread);
  } else {
    // No waiting threads, increment the value
    atomic_fetch_add(&(sem->value), 1);
    mthread_internal_spin_unlock(&(sem->thread_list_spinlock));
  }

  return 0;
}

int mthread_sem_getvalue(mthread_sem_t *sem, int *val) {
  if (sem == nullptr || val == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (!sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }
  *val = atomic_load(&(sem->value));
  return 0;
}

int mthread_sem_trylock(mthread_sem_t *sem) {
  if (sem == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (!sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }

  int current = atomic_load(&(sem->value));
  if (current > 0) {
    atomic_fetch_sub(&(sem->value), 1);
    return 0;
  }

  return MTHREAD_SEM_ERROR_ALREADY_LOCKED;
}

