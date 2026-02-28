#include <mthread.h>
#include <mthread_common_helpers.h>
#include <mthread_thread.h>
#include <mthread_vp_internal.h>

/* Q.1 : mthread_cond_init */
int mthread_cond_init(mthread_cond_t *cond) {
  if (cond == nullptr) {
    return MTHREAD_COND_ERROR_NULL;
  }
  if (cond->is_initialized) {
    return MTHREAD_COND_ERROR_INIT;
  }
  cond->is_initialized = true;
  atomic_flag_clear(&(cond->thread_list_spinlock));
  cond->thread_list.head = nullptr;
  cond->thread_list.tail = nullptr;
  return 0;
}

/* Q.2 : mthread_cond_wait */
int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mut) {
  if (cond == nullptr) {
    return MTHREAD_COND_ERROR_NULL;
  }
  if (!cond->is_initialized) {
    return MTHREAD_COND_ERROR_INIT;
  }
  if (!atomic_load(&(mut->is_locked))) {
    return MTHREAD_COND_ERROR_NOT_LOCKED;
  }

  mthread_internal_spin_lock(&(cond->thread_list_spinlock));

  /* Relâcher le mutex avant de se bloquer */
  mthread_mutex_unlock(mut);

  /* S'insérer dans la file d'attente de la condition */
  mthread_thread_t *self = mthread_self();
  mthread_list_item_t item;
  item.thread = self;
  insert_tail_in_list(&item, &cond->thread_list);

  /* Se bloquer (libère le spinlock de manière atomique) */
  mthread_block_self(&(cond->thread_list_spinlock));

  /* Au réveil : ré-acquérir le mutex */
  mthread_mutex_lock(mut);

  return 0;
}

/* Q.3 : mthread_cond_signal */
int mthread_cond_signal(mthread_cond_t *cond) {
  if (cond == nullptr) {
    return MTHREAD_COND_ERROR_NULL;
  }
  if (!cond->is_initialized) {
    return MTHREAD_COND_ERROR_INIT;
  }

  mthread_internal_spin_lock(&(cond->thread_list_spinlock));
  if (cond->thread_list.head != nullptr) {
    mthread_list_item_t *item = remove_head_in_list(&(cond->thread_list));
    mthread_internal_spin_unlock(&(cond->thread_list_spinlock));
    mthread_wake_thread(item->thread);
  } else {
    mthread_internal_spin_unlock(&(cond->thread_list_spinlock));
  }

  return 0;
}

/* Q.4 : mthread_cond_broadcast */
int mthread_cond_broadcast(mthread_cond_t *cond) {
  if (cond == nullptr) {
    return MTHREAD_COND_ERROR_NULL;
  }
  if (!cond->is_initialized) {
    return MTHREAD_COND_ERROR_INIT;
  }

  mthread_internal_spin_lock(&(cond->thread_list_spinlock));
  while (cond->thread_list.head != nullptr) {
    mthread_list_item_t *item = remove_head_in_list(&(cond->thread_list));
    mthread_wake_thread(item->thread);
  }
  mthread_internal_spin_unlock(&(cond->thread_list_spinlock));

  return 0;
}

/* Q.5 : mthread_cond_destroy */
int mthread_cond_destroy(mthread_cond_t *cond) {
  if (cond == nullptr) {
    return MTHREAD_COND_ERROR_NULL;
  }
  if (!cond->is_initialized) {
    return MTHREAD_COND_ERROR_INIT;
  }
  if (cond->thread_list.head != nullptr) {
    return MTHREAD_COND_ERROR_IN_USE;
  }
  cond->is_initialized = false;
  cond->thread_list.head = nullptr;
  cond->thread_list.tail = nullptr;
  return 0;
}
