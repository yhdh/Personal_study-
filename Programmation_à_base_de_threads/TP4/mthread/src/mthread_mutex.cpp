#include <mthread.h>
#include <mthread_common_helpers.h>
#include <mthread_thread.h>
#include <mthread_vp_internal.h>
#include <mthread_vp_threads.h>

int mthread_mutex_lock(mthread_mutex_t *lock) {
  if (lock == nullptr) {
    return MTHREAD_MUTEX_ERROR_NULL;
  } else {
    if (!lock->is_initialized) {
      int res;
      res = mthread_mutex_init(nullptr, lock);
      if (res != 0) {
        return MTHREAD_MUTEX_ERROR_INIT;
      }
    }
    assert(lock != nullptr);
    assert(lock->is_initialized == true);
    mthread_thread_t *current_thread;
    current_thread = mthread_self();
    mthread_internal_spin_lock(&(lock->thread_list_spinlock));
    if (!lock->is_locked) {
	  lock->is_locked = true;
      lock->owner = current_thread;
      mthread_internal_spin_unlock(&(lock->thread_list_spinlock));
      mthread_log("MUTEX", "Lock %p hold %p\n", lock->owner, lock);
      return 0;
    } else {
      mthread_list_item_t item;
      item.thread = current_thread;
      insert_tail_in_list(&item, &lock->thread_list);

      // Block in the scheduler and release the spinlock
      mthread_block_self(lock);
    }
  }
  return 0;
}
int mthread_mutex_unlock(mthread_mutex_t *lock) {
  mthread_thread_t *current_thread;
  if (lock == nullptr) {
    return MTHREAD_MUTEX_ERROR_NULL;
  } else {
    if (!lock->is_initialized) {
      int res;
      res = mthread_mutex_init(nullptr, lock);
      if (res != 0) {
        return MTHREAD_MUTEX_ERROR_INIT;
      }
    }
    if (!atomic_load(&(lock->is_locked))) {
      return MTHREAD_MUTEX_ERROR_NOT_LOCKED;
    }
    assert(lock != nullptr);
    assert(lock->is_initialized == true);
    current_thread = mthread_self();
    if (current_thread != lock->owner) {
      return MTHREAD_MUTEX_ERROR_OWNER;
    }
    if (lock->thread_list.head != nullptr) {
      mthread_internal_spin_lock(&(lock->thread_list_spinlock));
      mthread_list_item_t *item;
      item = remove_head_in_list(&(lock->thread_list));
      mthread_internal_spin_unlock(&(lock->thread_list_spinlock));

      // wake thread
      lock->owner = item->thread;
      mthread_log("MUTEX", "Unlock %p hold %p\n", lock->owner, lock);
      mthread_wake_thread(item->thread);
    } else {
      atomic_store(&(lock->is_locked), false);
    }
  }
  return 0;
}
int mthread_mutex_trylock(mthread_mutex_t *lock) {
  bool unlocked = false;
  if (lock == nullptr) {
    return MTHREAD_MUTEX_ERROR_NULL;
  } else {
    if (!lock->is_initialized) {
      int res;
      res = mthread_mutex_init(nullptr, lock);
      if (res != 0) {
        return MTHREAD_MUTEX_ERROR_INIT;
      }
    }
    assert(lock != nullptr);
    assert(lock->is_initialized == true);
    if (atomic_compare_exchange_strong(&(lock->is_locked), &unlocked, true)) {
      lock->owner = mthread_self();
      mthread_log("MUTEX", "TryLock %p hold %p\n", lock->owner, lock);
      return 0;
    } else {
      return MTHREAD_MUTEX_ERROR_ALREADY_LOCKED;
    }
  }
  return 0;
}

int mthread_mutex_init(mthread_mutex_attr_t *attr, mthread_mutex_t *lock) {
  if (lock == nullptr) {
    return MTHREAD_MUTEX_ERROR_NULL;
  }
  if (attr != nullptr) {
    not_implemented();
  }
  if (lock->is_initialized) {
    return MTHREAD_MUTEX_ERROR_INIT;
  }
  { lock->is_initialized = true; }
  return 0;
}

int mthread_mutex_destroy(mthread_mutex_t *lock) {
  if (lock == nullptr) {
    return MTHREAD_MUTEX_ERROR_NULL;
  }
  if (!lock->is_initialized) {
    return MTHREAD_MUTEX_ERROR_INIT;
  }
  if (atomic_load(&(lock->is_locked))) {
    return MTHREAD_MUTEX_ERROR_ALREADY_LOCKED;
  }
  if (lock->thread_list.head != nullptr) {
    return MTHREAD_MUTEX_ERROR_ALREADY_LOCKED;
  }
  lock->is_initialized = false;
  lock->is_locked = false;
  lock->owner = nullptr;
  lock->thread_list.head = nullptr;
  lock->thread_list.tail = nullptr;
  return 0;
}
