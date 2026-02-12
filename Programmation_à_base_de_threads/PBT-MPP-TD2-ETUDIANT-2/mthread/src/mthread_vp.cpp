#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>

#include <mthread.h>
#include <mthread_thread.h>
#include <mthread_vp_allocator.h>
#include <mthread_vp_internal.h>
#include <mthread_vp_threads.h>
#ifdef MTHREAD_HAVE_CXX_ABI
#include <cxxabi.h>
#endif

// Include schedulers
#include <mthread_scheduler_empty.h>
#include <mthread_scheduler_fifo.h>
#include <mthread_scheduler_fifo_class.h>
#include <mthread_scheduler_helpers.h>

// NOLINTBEGIN
template <typename Sched, typename Lock>
static inline void internal_print_available_threads() {
  vp_list<Sched, Lock>.vp_thread.lock();
  fprintf(stderr, "\n>>>   ");
  for (struct mthread_vp_s<Sched> *vp : vp_list<Sched, Lock>.vp_vector) {
    fprintf(stderr, "\n========== VP %d ==========\n", vp->id);
    fprintf(stderr, "\tPrint threads vp %d available (CREATED):\n", vp->id);
    mthread_print_list(vp->available_thread, vp->idle);
    vp->scheduler.print_scheduler_status(vp);
  }
  fprintf(stderr, "<<<\n");
  vp_list<Sched, Lock>.vp_thread.unlock();
}
// NOLINTEND

template <typename Sched>
static thread_local struct mthread_vp_s<Sched> *mthread_current_vp = nullptr;

// NOLINTBEGIN
constexpr size_t MTHREAD_LOG_PART = 15;
constexpr size_t MTHREAD_LOG_BUFFER_SIZE = 4096;
template <typename Sched>
static int internal_mthread_log(const char *part, const char *format, ...) {
  static thread_local char *msg = nullptr;
  static thread_local char *part2 = nullptr;
  va_list ap;
  size_t len;
  int res;

  if (msg == nullptr) {
    msg = (char *)malloc(MTHREAD_LOG_BUFFER_SIZE * sizeof(char));
    part2 = (char *)malloc((MTHREAD_LOG_PART + 10) * sizeof(char));
  }
  assert(msg != nullptr);
  assert(part2 != nullptr);

  for (size_t i = 0; i < MTHREAD_LOG_PART; i++) {
    part2[i] = ' ';
  }
  len = strlen(part);
  if (len >= MTHREAD_LOG_PART) {
    len = MTHREAD_LOG_PART;
  }
  memcpy(part2, part, len);
  part2[MTHREAD_LOG_PART] = '\0';

  if (mthread_current_vp<Sched> == nullptr) {
    snprintf(msg, MTHREAD_LOG_BUFFER_SIZE,
             "[LWP %02d Thread %p (Binding %3d) %s INFO:] %s", -1, nullptr, -1,
             part2, format);
  } else {
    snprintf(msg, MTHREAD_LOG_BUFFER_SIZE,
             "[LWP %02d Thread %p (Binding %3d) %s INFO:] %s",
             mthread_current_vp<Sched>->id,
             mthread_current_vp<Sched>->current_thread,
             mthread_current_vp<Sched>->current_thread->attr.vp, part2, format);
  }

  va_start(ap, format);
  res = vfprintf(stderr, msg, ap);
  va_end(ap);
  fflush(stderr);
  return res;
}
// NOLINTEND

template <typename Sched>
static inline void internal_mthread_yield(struct mthread_vp_s<Sched> *vp) {
  assert(vp->current_thread != nullptr);
  assert(vp->next_thread == nullptr);

  // Select new thread
  vp->scheduler.find_next_thread(vp);

  // Schedule new thread
  if (vp->next_thread == nullptr) {
    // Need to schedule idle func
    vp->next_thread = vp->idle;
  }
  assert(vp->next_thread->status == mthread_running);
  if (vp->current_thread != vp->next_thread) {
    mthread_thread_t *tmp_cur = vp->current_thread;
    mthread_thread_t *tmp_next = vp->next_thread;
    vp->current_thread = vp->next_thread;
    vp->next_thread = nullptr;
    swapcontext(&(tmp_cur->uc), &(tmp_next->uc));
  } else {
    vp->next_thread = nullptr;
  }

  // Get current VP, vp can be false with migrations
  vp = mthread_current_vp<Sched>;

  if (vp->registered_spinlock != nullptr) {
    mthread_internal_spin_unlock(vp->registered_spinlock);
    vp->registered_spinlock = nullptr;
  }

  // Reinsert previous thread in lists
  vp->scheduler.reschedule_current(vp);
}

template <typename Sched> static inline void internal_mthread_yield() {
  internal_mthread_yield(mthread_current_vp<Sched>);
}

template <typename Sched>
static inline mthread_thread_t *
internal_mthread_self(struct mthread_vp_s<Sched> *vp) {
  return vp->current_thread;
}

template <typename Sched>
static inline mthread_thread_t *internal_mthread_self() {
  return internal_mthread_self(mthread_current_vp<Sched>);
}

template <typename Sched>
static inline void internal_mthread_block_self(struct mthread_vp_s<Sched> *vp,
                                               mthread_mutex_t *lock) {
  vp->current_thread->status = mthread_blocked;

  if (vp->registered_spinlock != nullptr) {
    mthread_internal_spin_unlock(vp->registered_spinlock);
    vp->registered_spinlock = nullptr;
  }

  vp->registered_spinlock = &(lock->thread_list_spinlock);
  internal_mthread_yield(vp);
}

template <typename Sched>
static inline void internal_mthread_block_self(struct mthread_vp_s<Sched> *vp,
                                              atomic_flag *spinlock)
{
  vp->current_thread->status = mthread_blocked;

  if (vp->registered_spinlock != nullptr) {
    mthread_internal_spin_unlock(vp->registered_spinlock);
    vp->registered_spinlock = nullptr;
  }
  
  vp->registered_spinlock = spinlock;
  internal_mthread_yield(vp);

}

template <typename Sched>
static inline void internal_mthread_block_self(mthread_mutex_t *lock) {
  internal_mthread_block_self(mthread_current_vp<Sched>, lock);
}

template <typename Sched>
static inline void internal_mthread_block_self(atomic_flag *spinlock) {
  internal_mthread_block_self(mthread_current_vp<Sched>, spinlock);
}

template <typename Sched>
static inline void internal_mthread_wake_thread(mthread_thread_t *thread) {
  struct mthread_vp_s<Sched> *vp;
  vp = mthread_current_vp<Sched>;

  mthread_log("SCHED", "Wait to reinsert blocked thread %p\n", thread);
  // Wait until the thread has been removed from ready list
  while (thread->status != mthread_blocked_ready) {
    internal_mthread_yield<Sched>();
  }

  mthread_log("SCHED", "Reinsert blocked thread %p\n", thread);
  thread->status = mthread_running;
  vp->scheduler.insert_new_thread(vp, thread);
}

template <typename Sched>
static inline struct mthread_vp_s<Sched> *
idle_func(struct mthread_vp_s<Sched> *vp) {
  mthread_current_vp<Sched> = vp;

  while (true) {
    internal_mthread_yield<Sched>();
  }

  not_reachable();
  return vp;
}

/// @brief Trampoline function for new threads
/// @tparam Sched Scheduler name
/// @param current Pointer to the thread to start
template <typename Sched>
static void gen_start_func(mthread_thread_t *current) {
  assert(mthread_current_vp<Sched> != nullptr);

  internal_mthread_yield<Sched>();

  current->res = current->func(current->arg);
  assert(current->status == mthread_running);
  current->status = mthread_zombie;
  internal_mthread_yield<Sched>();
}

template <typename Sched, typename Alloc>
static inline mthread_thread_t *
create_thread(const mthread_attr_t *internal_attr, mthread_vp_s<Sched> *vp,
              void *(*func)(void *), void *arg) {
  const mthread_attr_t default_attr;
  auto *current_thread =
      new (vp_allocator<Alloc>->thread_alloc()) mthread_thread_t();

  assert(current_thread->status == mthread_not_initialized);

  current_thread->func = func;
  current_thread->arg = arg;
  // If attr == nullptr use the default values for attr
  if (internal_attr == nullptr) {
    current_thread->attr = default_attr;
  } else {
    current_thread->attr = *internal_attr;
  }

  // Get the stack for the thread to create
  if (current_thread->attr.stack != nullptr) {
    current_thread->attr.user_stack = true;
  } else {
    current_thread->attr.user_stack = false;
  }

  /* fetch current context */
  if (getcontext(&(current_thread->uc)) != 0)
    return nullptr;

  // If the stack is not provided and stack_size is standard then allocate a
  // buffered stack
  if (current_thread->attr.stack == default_attr.stack) {
    // Custom size not compatibles with buffered allocator
    if (current_thread->attr.stack_size != default_attr.stack_size) {
      current_thread->attr.stack =
          (void *)vp_allocator<Alloc>->custom_stack_alloc(
              current_thread->attr.stack_size);
      current_thread->attr.buffered_stack = false;
    } else {
      current_thread->attr.stack = (void *)vp_allocator<Alloc>->stack_alloc(
          current_thread->attr.stack_size);
      current_thread->attr.buffered_stack = true;
    }
  } else {
    current_thread->attr.buffered_stack = false;
  }

  assert(current_thread->attr.stack != nullptr);

  /* remove parent link */
  current_thread->uc.uc_link = nullptr;

  /* configure new stack */
  current_thread->uc.uc_stack.ss_sp = current_thread->attr.stack;
  current_thread->uc.uc_stack.ss_size = current_thread->attr.stack_size;
  current_thread->uc.uc_stack.ss_flags = 0;

  if (func != nullptr) {
    /* configure startup function (with one argument) */
    makecontext(&(current_thread->uc), (void (*)())gen_start_func<Sched>, 1 + 1,
                (void *)current_thread);
  }

  current_thread->status = mthread_running;
  vp->available_thread.push_back(current_thread);

  assert(vp->idle_func != nullptr);
  if (func != vp->idle_func) {
    if (current_thread->attr.vp == -1) {
      current_thread->attr.vp = vp->scheduler.loadbalancer(vp, current_thread);
    }
    vp->scheduler.insert_new_thread(vp, current_thread);
  }

  return current_thread;
}

template <typename Sched, typename Alloc>
void internal_mthread_garbage_collect_threads(struct mthread_vp_s<Sched> *vp) {
  // Garbage collect zombie threads
  for (auto it = vp->available_thread.begin();
       it != vp->available_thread.end();) {
    mthread_thread_t *t = (*it);
    if (t->status == mthread_zombie_joined) {
      mthread_log("SCHED", "Erase %p\n", t);

      if (t->attr.stack != nullptr) {
        if (!t->attr.user_stack) {
          if (t->attr.buffered_stack) {
            vp_allocator<Alloc>->stack_free(t->attr.stack);
          } else {
            vp_allocator<Alloc>->custom_stack_free(t->attr.stack);
          }
        }
        t->attr.stack = nullptr;
      }

      // Use a custom thread allocator with placement, can not call delete
      t->~mthread_thread_t();
      vp_allocator<Alloc>->thread_free(t);

      it = vp->available_thread.erase(it);
    } else {
      ++it;
    }
  }
}

template <typename Sched, typename Alloc>
static inline mthread_thread_t *
create_user_thread(mthread_attr_t *attr, void *(*func)(void *), void *arg) {
  struct mthread_vp_s<Sched> *vp = mthread_current_vp<Sched>;
  mthread_thread_t *res = create_thread<Sched, Alloc>(attr, vp, func, arg);
  assert(res != nullptr);
  return res;
}

template <typename Sched, typename Alloc>
static int internal_mthread_join(mthread_thread_t *th, void **res) {
  struct mthread_vp_s<Sched> *vp = mthread_current_vp<Sched>;

  while (th->status != mthread_zombie_joinable) {
    internal_mthread_yield<Sched>();
  }

  *res = th->res;
  th->status = mthread_zombie_joined;

  internal_mthread_garbage_collect_threads<Sched, Alloc>(vp);
  return 0;
}

template <typename Sched, typename Lock, typename Alloc>
static inline mthread_vp_s<Sched> *create_vp_idle(int i) {
  mthread_attr_t default_attr;

  auto *vp = new mthread_vp_s<Sched>();
  vp_allocator<Alloc> = new mthread_threads_buffered_allocator<Alloc>;

  // Generic init
  vp->id = i;
  vp->next_thread = nullptr;
  default_attr.vp = i;
  vp->idle_func = (void *(*)(void *))idle_func<Sched>;

  // Add to the list
  vp_list<Sched, Lock>.vp_thread.lock();
  assert((vp_list<Sched, Lock>.vp_vector.size()) != 0);
  vp_list<Sched, Lock>.vp_vector[i] = vp;
  vp_list<Sched, Lock>.vp_list.push_back(vp);
  vp_list<Sched, Lock>.vp_thread.unlock();

  // After this point all VPs are started, just have to launch idle thread
  vp_list<Sched, Lock>.vp_thread.barrier_wait();

  mthread_thread_t *idle = create_thread<Sched, Alloc>(
      &default_attr, vp, (void *(*)(void *))idle_func<Sched>, (void *)vp);
  fatal_error(idle != nullptr);
  vp->idle = idle;

  return vp;
}

typedef struct {
  int id;
} thread_vp_struct_t;

template <typename Sched, typename Lock, typename Alloc>
[[noreturn]] static void *pthread_vp_thread(thread_vp_struct_t *arg) {
  mthread_vp_s<Sched> *vp = create_vp_idle<Sched, Lock, Alloc>(arg->id);
  delete arg;

  mthread_current_vp<Sched> = vp;
  vp->current_thread = vp->idle;

  setcontext(&(vp->idle->uc));
  not_reachable();
  abort();
}

template <typename Sched, typename Lock, typename Alloc>
static inline void create_vp(int i) {
  if (i == 0) {
    mthread_attr_t default_attr;
    mthread_vp_s<Sched> *vp = create_vp_idle<Sched, Lock, Alloc>(i);

    default_attr.vp = 0;

    mthread_thread_t *current =
        create_thread<Sched, Alloc>(&default_attr, vp, nullptr, nullptr);
    assert(current != nullptr);

    mthread_current_vp<Sched> = vp;

    // Schedule main thread instead of idle
    vp->current_thread = current;
  } else {
    auto *arg = new thread_vp_struct_t();
    arg->id = i;
    vp_list<Sched, Lock>.vp_thread.thread_create(
        (void *(*)(void *))pthread_vp_thread<Sched, Lock, Alloc>, arg);
  }
}

// Block_self
void internal_default_mthread_block_self(
    [[maybe_unused]] mthread_mutex_t *lock) {
  not_reachable();
}

void internal_default_mthread_block_self(
    [[maybe_unused]] atomic_flag *spinlock) {
  not_reachable();
}

void (*mthread_block_self_ptr)(mthread_mutex_t *) =
    internal_default_mthread_block_self;
void mthread_block_self(mthread_mutex_t *lock) { mthread_block_self_ptr(lock); }

void (*mthread_block_self_spinlock_ptr)(atomic_flag *) =
    internal_default_mthread_block_self;
void mthread_block_self(atomic_flag *spinlock) { mthread_block_self_spinlock_ptr(spinlock); }

// Wake_thread
void internal_default_mthread_wake_thread(
    [[maybe_unused]] mthread_thread_t *thread) {
  not_reachable();
}
void (*mthread_wake_thread_ptr)(mthread_thread_t *) =
    internal_default_mthread_wake_thread;
void mthread_wake_thread(mthread_thread_t *thread) {
  mthread_wake_thread_ptr(thread);
}

// thread_self
static mthread_thread_t *internal_default_mthread_self() { return nullptr; }
static mthread_thread_t *(*mthread_self_ptr)() = internal_default_mthread_self;
extern mthread_t mthread_self() { return mthread_self_ptr(); }

// thread_yield
static void internal_default_mthread_yield() {}
void (*mthread_yield_ptr)() = internal_default_mthread_yield;
void mthread_yield() { mthread_yield_ptr(); }

// print_available_threads
void (*print_available_threads_ptr)() = internal_print_available_threads<
    empty_scheduler_t, mthread_threads_pthread_spinlock>;
void print_available_threads() { print_available_threads_ptr(); }

// mthread_create_thread
static mthread_thread_t *
internal_default_mthread_create_thread([[maybe_unused]] mthread_attr_t *attr,
                                       [[maybe_unused]] void *(*func)(void *),
                                       [[maybe_unused]] void *arg) {
  return nullptr;
}
mthread_thread_t *(*mthread_create_thread_ptr)(
    mthread_attr_t *attr, void *(*func)(void *),
    void *arg) = internal_default_mthread_create_thread;
mthread_t mthread_create_thread(mthread_attr_t *attr, void *(*func)(void *),
                                void *arg) {
  return mthread_create_thread_ptr(attr, func, arg);
}

// mthread_join
static int internal_default_mthread_join(mthread_thread_t *, void **) {
  return 1;
}
int (*mthread_join_ptr)(mthread_thread_t *,
                        void **) = internal_default_mthread_join;
int mthread_join(mthread_t th, void **res) { return mthread_join_ptr(th, res); }

// mthread_log
int (*mthread_log)(const char *part, const char *format,
                   ...) = internal_mthread_log<empty_scheduler_t>;

// Initialize new Scheduler
template <typename Sched, typename Lock, typename Alloc>
static inline void mthread_init_scheduler(int nb_vp) {
  if (nb_vp == 0) {
    return;
  }

  vp_list<Sched, Lock>.vp_thread.barrier_init(nb_vp);

  assert((vp_list<Sched, Lock>.vp_vector.size()) == 0);

  vp_list<Sched, Lock>.vp_vector.resize(nb_vp);

#ifdef MTHREAD_HAVE_CXX_ABI
  int status = 0;
  char *realname =
      abi::__cxa_demangle(typeid(Sched).name(), nullptr, nullptr, &status);
#else
  char *realname = (char *)"";
#endif

  fprintf(stderr, "Init mthread in mode %s (%s)\n", MTHREAD_BUILD_TYPE,
          realname);

  mthread_create_thread_ptr = (struct mthread_thread_s *
                               (*)(mthread_attr_t *, void *(*)(void *), void *))
      create_user_thread<Sched, Alloc>;
  mthread_yield_ptr = internal_mthread_yield<Sched>;
  mthread_self_ptr = internal_mthread_self<Sched>;
  print_available_threads_ptr = internal_print_available_threads<Sched, Lock>;
  mthread_log = internal_mthread_log<Sched>;
  mthread_join_ptr = internal_mthread_join<Sched, Alloc>;
  mthread_block_self_ptr = internal_mthread_block_self<Sched>;
  mthread_block_self_spinlock_ptr = internal_mthread_block_self<Sched>;
  mthread_wake_thread_ptr = internal_mthread_wake_thread<Sched>;

  for (int i = 1; i < nb_vp; i++) {
    create_vp<Sched, Lock, Alloc>(i);
  }

  // Must be the last to be created to avoid deadlock in barrier
  create_vp<Sched, Lock, Alloc>(0);
}

template <template <typename, typename, typename> typename Sched,
          typename SchedLock, typename Lock, typename LoadBalancer,
          typename Alloc>
void internal_mthread_init_scheduler(int nb_vp) {
  mthread_init_scheduler<Sched<SchedLock, Lock, LoadBalancer>, Lock, Alloc>(
      nb_vp);
}

template <typename SchedLock, typename Lock, typename LoadBalancer,
          typename Alloc>
void internal_mthread_init_scheduler_fifo(int nb_vp) {
  mthread_init_scheduler<struct fifo_scheduler_s<SchedLock, Lock, LoadBalancer>,
                         Lock, Alloc>(nb_vp);
}

[[maybe_unused]] void mthread_init_scheduler_fifo(int nb_vp) {
  internal_mthread_init_scheduler_fifo<
      mthread_threads_pthread_spinlock,
      mthread_threads_pthread<mthread_threads_pthread_spinlock,
                              mthread_threads_pthread_barrier>,
      mthread_dummy_loadbalancer, mthread_item_buffer_allocator>(nb_vp);
}

// Easier to have generic code with class than struct
void mthread_init_scheduler_fifo_class(int nb_vp) {
  internal_mthread_init_scheduler<
      fifo_class_scheduler_t, mthread_threads_pthread_spinlock,
      mthread_threads_pthread<mthread_threads_pthread_spinlock,
                              mthread_threads_pthread_barrier>,
      mthread_dummy_loadbalancer, mthread_item_buffer_allocator>(nb_vp);
}
