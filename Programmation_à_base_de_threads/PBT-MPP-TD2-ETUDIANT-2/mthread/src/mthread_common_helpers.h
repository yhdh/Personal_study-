#ifndef MTHREAD_COMMON_HELPERS_H

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <mthread.h>
#include <sched.h>

#ifdef HAVE_CXX_STACKTRACE
#include <iostream>
#include <stacktrace>

[[maybe_unused]] static void print_stacktrace() {
  std::cout << std::stacktrace::current();
}
#else
[[maybe_unused]] static void print_stacktrace() {}
#endif

#ifdef __cplusplus
extern "C" {
#endif
void mthread_abort();
#ifdef __cplusplus
}
#endif

/// @brief Function to use when it is not possible to reach this point
[[maybe_unused]] static void not_reachable() {
  fprintf(stderr, "FATAL not_reachable\n");
  print_stacktrace();
  mthread_abort();
}

/// @brief Function to use when the feature is not implemented yet
[[maybe_unused]] static void not_implemented() {
  fprintf(stderr, "FATAL not_implemented\n");
  print_stacktrace();
  mthread_abort();
}

/// @brief Sort of assert but not disabled by NDEBUG
/// @param val Assertion to test
[[maybe_unused]] static void fatal_error(bool val) {
  if (!val) {
    fprintf(stderr, "FATAL fatal_error\n");
    print_stacktrace();
    mthread_abort();
  }
}

/// @brief Insert item at the end of the list _list
/// @param item Item to insert
/// @param _list List to modify
[[maybe_unused]] static void insert_tail_in_list(mthread_list_item_t *item,
                                                 mthread_list_t *_list) {
  assert(_list != nullptr);
  if (item == nullptr) {
    return;
  }
  item->next = nullptr;
  if (_list->head == nullptr) {
    _list->head = item;
    _list->tail = item;
  } else {
    volatile mthread_list_item_t *prev_tail;
    prev_tail = _list->tail;
    prev_tail->next = item;
    _list->tail = item;
  }
}

/// @brief Remove and return the head of the list _list
/// @param _list List to modify
/// @return The first item
[[maybe_unused]] static mthread_list_item_t *
remove_head_in_list(mthread_list_t *_list) {
  mthread_list_item_t *item;
  if (_list == nullptr) {
    return nullptr;
  }
  if (_list->head == nullptr) {
    return nullptr;
  }
  item = (mthread_list_item_t *)_list->head;
  _list->head = item->next;
  if (item == _list->tail) {
    _list->tail = _list->head;
  }
  return item;
}

/// @brief Spin on the lock thread_list_spinlock
/// @param thread_list_spinlock Pointer to the lock
[[maybe_unused]] static void
mthread_internal_spin_lock(atomic_flag *thread_list_spinlock) {
  while (atomic_flag_test_and_set(thread_list_spinlock)) {
    sched_yield();
  }
}

/// @brief Unlock the spinlock thread_list_spinlock
/// @param thread_list_spinlock Pointer to the lock
[[maybe_unused]] static void
mthread_internal_spin_unlock(atomic_flag *thread_list_spinlock) {
  atomic_flag_clear(thread_list_spinlock);
}

#define MTHREAD_COMMON_HELPERS_H
#endif
