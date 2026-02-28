/*
 * test_scheduler_unit_mutex_destroy.cpp
 * Q.22 — Test des fonctions mutex : destroy, trylock, MTHREAD_MUTEX_INITIALIZER
 *
 * Ce programme teste :
 *   1. mthread_mutex_init    (Q.16)
 *   2. mthread_mutex_lock    (Q.17)
 *   3. mthread_mutex_unlock  (Q.18)
 *   4. mthread_mutex_destroy (Q.19)
 *   5. mthread_mutex_trylock (Q.20)
 *   6. MTHREAD_MUTEX_INITIALIZER (Q.21)
 */

#include <cassert>
#include <cstdio>
#include <mthread.h>

int NB_VP = 1;
int NB_THS = 1;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for this test"
#endif

#define Check_val(val)                                                         \
  {                                                                            \
    assert(res == (val));                                                       \
    if (res != (val)) {                                                        \
      return 1;                                                                \
    }                                                                          \
  }                                                                            \
  (void)(0)

/* ═══════════ Mutex initialisé statiquement (Q.21) ═══════════ */
static mthread_mutex_t static_mutex = MTHREAD_MUTEX_INITIALIZER;

static int shared_counter = 0;

/* Thread function for concurrent mutex test */
void *thread_increment([[maybe_unused]] void *arg) {
  for (int i = 0; i < 100; i++) {
    mthread_mutex_lock(&static_mutex);
    shared_counter++;
    mthread_mutex_unlock(&static_mutex);
    mthread_yield();
  }
  return nullptr;
}

/* Thread function for trylock test */
static mthread_mutex_t trylock_mutex;

void *thread_trylock_test([[maybe_unused]] void *arg) {
  /* Le mutex est déjà verrouillé par le main → trylock doit échouer */
  int res = mthread_mutex_trylock(&trylock_mutex);
  assert(res == MTHREAD_MUTEX_ERROR_ALREADY_LOCKED);
  return nullptr;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  mthread_init_scheduler_test(NB_VP);
  int res;

  /* ═══════ TEST Q.16 : mthread_mutex_init ═══════ */
  fprintf(stderr, "=== Test mthread_mutex_init (Q.16) ===\n");
  {
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    fprintf(stderr, "  [OK] init normal\n");
  }
  {
    /* init(NULL) → erreur */
    res = mthread_mutex_init(nullptr, nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
    fprintf(stderr, "  [OK] init(NULL) → MTHREAD_MUTEX_ERROR_NULL\n");
  }
  {
    /* double init → erreur */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_init(nullptr, &m);
    Check_val(MTHREAD_MUTEX_ERROR_INIT);
    fprintf(stderr, "  [OK] double init → MTHREAD_MUTEX_ERROR_INIT\n");
  }
  fprintf(stderr, "=== Test mthread_mutex_init : PASS ===\n\n");

  /* ═══════ TEST Q.17 : mthread_mutex_lock ═══════ */
  fprintf(stderr, "=== Test mthread_mutex_lock (Q.17) ===\n");
  {
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_lock(&m);
    Check_val(0);
    fprintf(stderr, "  [OK] lock sur mutex libre\n");
    res = mthread_mutex_unlock(&m);
    Check_val(0);
  }
  {
    /* lock(NULL) → erreur */
    res = mthread_mutex_lock(nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
    fprintf(stderr, "  [OK] lock(NULL) → MTHREAD_MUTEX_ERROR_NULL\n");
  }
  {
    /* lock avec auto-init */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_lock(&m);
    Check_val(0);
    fprintf(stderr, "  [OK] lock avec auto-init\n");
    res = mthread_mutex_unlock(&m);
    Check_val(0);
  }
  fprintf(stderr, "=== Test mthread_mutex_lock : PASS ===\n\n");

  /* ═══════ TEST Q.18 : mthread_mutex_unlock ═══════ */
  fprintf(stderr, "=== Test mthread_mutex_unlock (Q.18) ===\n");
  {
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_lock(&m);
    Check_val(0);
    res = mthread_mutex_unlock(&m);
    Check_val(0);
    fprintf(stderr, "  [OK] lock puis unlock normal\n");
  }
  {
    /* unlock(NULL) → erreur */
    res = mthread_mutex_unlock(nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
    fprintf(stderr, "  [OK] unlock(NULL) → MTHREAD_MUTEX_ERROR_NULL\n");
  }
  {
    /* unlock sur mutex non verrouillé → erreur */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_unlock(&m);
    Check_val(MTHREAD_MUTEX_ERROR_NOT_LOCKED);
    fprintf(stderr, "  [OK] unlock non verrouillé → MTHREAD_MUTEX_ERROR_NOT_LOCKED\n");
  }
  fprintf(stderr, "=== Test mthread_mutex_unlock : PASS ===\n\n");

  /* ═══════ TEST Q.19 : mthread_mutex_destroy ═══════ */
  fprintf(stderr, "=== Test mthread_mutex_destroy (Q.19) ===\n");
  {
    /* Initialiser puis détruire un mutex libre */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_destroy(&m);
    Check_val(0);
    fprintf(stderr, "  [OK] destroy sur mutex non verrouillé\n");
  }
  {
    /* destroy(NULL) → erreur */
    res = mthread_mutex_destroy(nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
    fprintf(stderr, "  [OK] destroy(NULL) → MTHREAD_MUTEX_ERROR_NULL\n");
  }
  {
    /* destroy sur mutex non initialisé → erreur */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_destroy(&m);
    Check_val(MTHREAD_MUTEX_ERROR_INIT);
    fprintf(stderr, "  [OK] destroy non initialisé → MTHREAD_MUTEX_ERROR_INIT\n");
  }
  {
    /* destroy sur mutex verrouillé → erreur */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_lock(&m);
    Check_val(0);
    res = mthread_mutex_destroy(&m);
    Check_val(MTHREAD_MUTEX_ERROR_ALREADY_LOCKED);
    fprintf(stderr, "  [OK] destroy verrouillé → MTHREAD_MUTEX_ERROR_ALREADY_LOCKED\n");
    res = mthread_mutex_unlock(&m);
    Check_val(0);
  }
  fprintf(stderr, "=== Test mthread_mutex_destroy : PASS ===\n\n");

  /* ═══════ TEST Q.20 : mthread_mutex_trylock ═══════ */
  fprintf(stderr, "=== Test mthread_mutex_trylock (Q.20) ===\n");
  {
    /* trylock sur mutex libre → succès */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_trylock(&m);
    Check_val(0);
    fprintf(stderr, "  [OK] trylock sur mutex libre → succès\n");
    res = mthread_mutex_unlock(&m);
    Check_val(0);
  }
  {
    /* trylock sur mutex déjà verrouillé → échec */
    mthread_mutex_t m;
    m.is_initialized = false;
    res = mthread_mutex_init(nullptr, &m);
    Check_val(0);
    res = mthread_mutex_trylock(&m);
    Check_val(0);
    res = mthread_mutex_trylock(&m);
    Check_val(MTHREAD_MUTEX_ERROR_ALREADY_LOCKED);
    fprintf(stderr, "  [OK] trylock sur mutex pris → ALREADY_LOCKED\n");
    res = mthread_mutex_unlock(&m);
    Check_val(0);
  }
  {
    /* trylock(NULL) → erreur */
    res = mthread_mutex_trylock(nullptr);
    Check_val(MTHREAD_MUTEX_ERROR_NULL);
    fprintf(stderr, "  [OK] trylock(NULL) → MTHREAD_MUTEX_ERROR_NULL\n");
  }
  {
    /* trylock depuis un autre thread sur mutex verrouillé */
    trylock_mutex.is_initialized = false;
    res = mthread_mutex_init(nullptr, &trylock_mutex);
    Check_val(0);
    res = mthread_mutex_lock(&trylock_mutex);
    Check_val(0);

    mthread_thread_t *t1;
    mthread_create_thread(&t1, thread_trylock_test, nullptr);
    /* yield pour laisser le thread s'exécuter */
    mthread_yield();
    mthread_join(t1, nullptr);
    fprintf(stderr, "  [OK] trylock depuis autre thread sur mutex pris → échec\n");

    res = mthread_mutex_unlock(&trylock_mutex);
    Check_val(0);
    res = mthread_mutex_destroy(&trylock_mutex);
    Check_val(0);
  }
  fprintf(stderr, "=== Test mthread_mutex_trylock : PASS ===\n\n");

  /* ═══════ TEST Q.21 : MTHREAD_MUTEX_INITIALIZER ═══════ */
  fprintf(stderr, "=== Test MTHREAD_MUTEX_INITIALIZER (Q.21) ===\n");
  {
    /* lock/unlock direct sans appeler init */
    res = mthread_mutex_lock(&static_mutex);
    Check_val(0);
    fprintf(stderr, "  [OK] lock sur mutex statique sans init\n");
    res = mthread_mutex_unlock(&static_mutex);
    Check_val(0);
    fprintf(stderr, "  [OK] unlock sur mutex statique sans init\n");
  }
  {
    /* 5 threads concurrents avec exclusion mutuelle */
    shared_counter = 0;
    const int NB = 5;
    mthread_thread_t *threads[NB];
    for (int i = 0; i < NB; i++) {
      mthread_create_thread(&threads[i], thread_increment, nullptr);
    }
    for (int i = 0; i < NB; i++) {
      mthread_join(threads[i], nullptr);
    }
    /* Chaque thread fait 100 incréments → total = 500 */
    assert(shared_counter == NB * 100);
    fprintf(stderr, "  [OK] %d threads × 100 incréments → counter = %d (exclusion mutuelle OK)\n",
            NB, shared_counter);
  }
  fprintf(stderr, "=== Test MTHREAD_MUTEX_INITIALIZER : PASS ===\n\n");

  /* ═══════ RÉSUMÉ ═══════ */
  fprintf(stderr, "========================================\n");
  fprintf(stderr, "  TOUS LES TESTS Q.15-Q.22 PASSES !\n");
  fprintf(stderr, "========================================\n");

  return 0;
}
