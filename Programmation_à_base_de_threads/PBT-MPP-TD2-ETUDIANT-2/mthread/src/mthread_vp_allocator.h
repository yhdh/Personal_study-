#ifndef MTHREAD_VP_ALLOCATOR_H

#include <vector>

#include <cassert>
#include <malloc.h>
#include <mthread.h>
#include <mthread_thread.h>

#include <mthread_config.h>

#ifdef MTHREAD_USE_VALGRIND
#include <valgrind/valgrind.h>
#else
#define VALGRIND_STACK_REGISTER(a, b) (void(0))
#define VALGRIND_STACK_DEREGISTER(a) (void(0))
// #error
#endif

#define MTHREAD_BLOCK_NUMBER 100

class mthread_item_dummy_allocator {
public:
  explicit mthread_item_dummy_allocator(size_t size) { item_size = size; }

  mthread_item_dummy_allocator() { item_size = 0; }

  ~mthread_item_dummy_allocator() = default;

  void *item_alloc(size_t size) {
    void *res;
    if (item_size == 0)
      item_size = size;
    if (size == 0) {
      return nullptr;
    }
    assert(size > 0);
    res = malloc(size);
    assert(res != nullptr);
    return res;
  }

  void *item_alloc() {
    void *res;
    res = item_alloc(item_size);
    return res;
  }

  static void item_free(void *ptr) { free(ptr); }

private:
  size_t item_size = 0;
};

#if defined(HAVE_FLAG_SANITIZER_address)
using mthread_item_buffer_allocator = mthread_item_dummy_allocator;
#else
class mthread_item_buffer_allocator {
public:
  explicit mthread_item_buffer_allocator(size_t size) {
    item_vector.resize(MTHREAD_BLOCK_NUMBER);
    item_size = size;
  }

  mthread_item_buffer_allocator() {
    item_vector.resize(MTHREAD_BLOCK_NUMBER);
    item_size = 0;
  }

  ~mthread_item_buffer_allocator() {
    for (int i = 0; i < item_vector_pos; i++) {
      free(item_vector[i]);
    }
    item_vector.resize(0);
    item_size = 0;
  }

  void *item_alloc(size_t size) {
    void *res;
    if (item_size == 0)
      item_size = size;
    if (size == 0) {
      return nullptr;
    }
    assert(size > 0);
    assert(size == item_size);
    if (item_vector_pos > 0) {
      res = item_vector[item_vector_pos - 1];
      item_vector[item_vector_pos] = nullptr;
      item_vector_pos--;
    } else {
      res = malloc(size);
    }
    assert(res != nullptr);
    return res;
  }

  void *item_alloc() {
    void *res;
    res = item_alloc(item_size);
    return res;
  }

  void item_free(void *ptr) {
    if (item_vector_pos < (MTHREAD_BLOCK_NUMBER - 1)) {
      item_vector[item_vector_pos] = ptr;
      item_vector_pos++;
    } else {
      fprintf(stderr, "Free %p\n", ptr);
      free(ptr);
    }
  }

private:
  int item_vector_pos = 0;
  size_t item_size = 0;
  std::vector<void *> item_vector;
};
#endif

// Buffered Allocator
template <typename T> class mthread_buffered_allocator {
public:
  typedef size_t size_type;
  typedef T *pointer;
  [[maybe_unused]] typedef const T *const_pointer;
  using value_type [[maybe_unused]] = T;

  template <typename internal_Tp1> struct [[maybe_unused]] rebind {
    [[maybe_unused]] typedef mthread_buffered_allocator<internal_Tp1> other;
  };

  [[maybe_unused]] pointer
  allocate(size_type n, [[maybe_unused]] const void *hint = nullptr) {
    return (T *)buff_alloc.item_alloc(n * sizeof(T));
  }

  [[maybe_unused]] void deallocate(pointer p, [[maybe_unused]] size_type n) {
    buff_alloc.item_free(p);
  }

  mthread_buffered_allocator() noexcept = default;
  mthread_buffered_allocator(
      [[maybe_unused]] const mthread_buffered_allocator &a) noexcept {}
  template <class U>
  explicit mthread_buffered_allocator(
      [[maybe_unused]] const mthread_buffered_allocator<U> &a) noexcept {}
  ~mthread_buffered_allocator() noexcept = default;

private:
  mthread_item_buffer_allocator buff_alloc;
};

// Returns true if allocators b and a can be safely interchanged. Safely
// interchanged means that b could be used to deallocate storage obtained
// through a, and vice versa.
template <class T>
inline bool
operator==([[maybe_unused]] const mthread_buffered_allocator<T> &a,
           [[maybe_unused]] const mthread_buffered_allocator<T> &b) {
  return true;
}
// Returns !(a == b)
template <class T>
inline bool operator!=(const mthread_buffered_allocator<T> &a,
                       const mthread_buffered_allocator<T> &b) {
  return !(a == b);
}

// Regular Allocator
template <typename T> class mthread_regular_allocator {
public:
  typedef size_t size_type;
  typedef T *pointer;
  [[maybe_unused]] typedef const T *const_pointer;
  using value_type [[maybe_unused]] = T;

  template <typename internal_Tp1> struct [[maybe_unused]] rebind {
    [[maybe_unused]] typedef mthread_regular_allocator<internal_Tp1> other;
  };

  [[maybe_unused]] pointer
  allocate(size_type n, [[maybe_unused]] const void *hint = nullptr) {
    return (T *)malloc(n * sizeof(T));
  }

  [[maybe_unused]] void deallocate(pointer p, [[maybe_unused]] size_type n) {
    free(p);
  }

  mthread_regular_allocator() noexcept = default;

  mthread_regular_allocator(
      [[maybe_unused]] const mthread_regular_allocator &a) noexcept = default;
  template <class U>
  explicit mthread_regular_allocator(
      [[maybe_unused]] const mthread_regular_allocator<U> &a) noexcept {}
  ~mthread_regular_allocator() noexcept = default;
};

// Returns true if allocators b and a can be safely interchanged. Safely
// interchanged means that b could be used to deallocate storage obtained
// through a, and vice versa.
template <class T>
inline bool operator==([[maybe_unused]] const mthread_regular_allocator<T> &a,
                       [[maybe_unused]] const mthread_regular_allocator<T> &b) {
  return true;
}
// Returns !(a == b)
template <class T>
inline bool operator!=(const mthread_regular_allocator<T> &a,
                       const mthread_regular_allocator<T> &b) {
  return !(a == b);
}

template <typename Alloc> class mthread_threads_buffered_allocator {
public:
  void *stack_alloc(size_t size) {
    void *res;
    res = stack_buffer->item_alloc(size);
    VALGRIND_STACK_REGISTER(res, ((char *)res) + size);
    return res;
  }
  void stack_free(void *ptr) {
    VALGRIND_STACK_DEREGISTER(ptr);
    stack_buffer->item_free(ptr);
  }

  void *custom_stack_alloc(size_t size) {
    void *res;
    res = malloc(size);
    VALGRIND_STACK_REGISTER(res, ((char *)res) + size);
    return res;
  }
  void custom_stack_free(void *ptr) {
    VALGRIND_STACK_DEREGISTER(ptr);
    free(ptr);
  }

  mthread_thread_t *thread_alloc() {
    auto *res = (mthread_thread_t *)thread_buffer->item_alloc();
    res->status = mthread_not_initialized;
    return res;
  }
  void thread_free(mthread_thread_t *ptr) { thread_buffer->item_free(ptr); }

  mthread_threads_buffered_allocator() {
    mthread_attr_t default_attr;
    stack_buffer = new Alloc(default_attr.stack_size);
    thread_buffer = new Alloc(sizeof(mthread_thread_t));
  }

  ~mthread_threads_buffered_allocator() {
    delete stack_buffer;
    delete thread_buffer;
  }

private:
  Alloc *stack_buffer;
  Alloc *thread_buffer;
};

template <typename Alloc>
static thread_local mthread_threads_buffered_allocator<Alloc> *vp_allocator;

#define MTHREAD_VP_ALLOCATOR_H
#endif
