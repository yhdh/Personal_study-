/**
 * Q.6 : Programme d'exemple testant les 5 fonctions de condition variable
 *   - mthread_cond_init
 *   - mthread_cond_wait
 *   - mthread_cond_signal
 *   - mthread_cond_broadcast
 *   - mthread_cond_destroy
 *
 * Scénario :
 *   3 threads "worker" attendent sur une condition variable.
 *   1 thread "signaler" réveille 1 worker avec cond_signal.
 *   1 thread "broadcaster" réveille les 2 restants avec cond_broadcast.
 *   Le main vérifie les résultats après join.
 */

#include <cassert>
#include <cstdio>
#include <mthread.h>
#include <unistd.h>

/* ---------- Données partagées ---------- */

static mthread_mutex_t mutex;
static mthread_cond_t  cond;
static volatile int    woken = 0;   /* compteur de workers réveillés */

/* ---------- Thread worker : attend sur la condition ---------- */

void *worker([[maybe_unused]] void *arg) {
  long id = (long)arg;

  mthread_mutex_lock(&mutex);
  mthread_log("WORK", "Worker %ld : en attente sur la condition\n", id);
  mthread_cond_wait(&cond, &mutex);
  woken++;
  mthread_log("WORK", "Worker %ld : réveillé (woken=%d)\n", id, woken);
  mthread_mutex_unlock(&mutex);

  return nullptr;
}

/* ---------- Thread signaler : réveille 1 worker ---------- */

void *signaler([[maybe_unused]] void *arg) {
  sleep(1);   /* laisser les workers s'enregistrer */

  mthread_mutex_lock(&mutex);
  mthread_log("SIG ", "cond_signal → réveille 1 worker\n");
  int res = mthread_cond_signal(&cond);
  assert(res == 0);
  mthread_mutex_unlock(&mutex);

  return nullptr;
}

/* ---------- Thread broadcaster : réveille tous les workers restants ---------- */

void *broadcaster([[maybe_unused]] void *arg) {
  sleep(2);   /* laisser le signaler agir d'abord */

  mthread_mutex_lock(&mutex);
  mthread_log("BCAST", "cond_broadcast → réveille les workers restants\n");
  int res = mthread_cond_broadcast(&cond);
  assert(res == 0);
  mthread_mutex_unlock(&mutex);

  return nullptr;
}

/* ---------- Programme principal ---------- */

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  const int NB_VP      = 2;
  const int NB_WORKERS = 3;

  mthread_init_scheduler_fifo_class(NB_VP);
  mthread_log("MAIN", "=== Q.6 : Test des conditions variables ===\n");

  /* ================================================================
   *  Test 1 : mthread_cond_init
   * ================================================================ */
  {
    int res;

    /* init normal */
    res = mthread_cond_init(&cond);
    assert(res == 0);
    mthread_log("MAIN", "[init]  cond_init(&cond)        = %d  OK\n", res);

    /* init sur nullptr */
    res = mthread_cond_init(nullptr);
    assert(res == MTHREAD_COND_ERROR_NULL);
    mthread_log("MAIN", "[init]  cond_init(nullptr)      = %d  OK (ERROR_NULL)\n", res);

    /* double init */
    res = mthread_cond_init(&cond);
    assert(res == MTHREAD_COND_ERROR_INIT);
    mthread_log("MAIN", "[init]  cond_init(&cond) x2     = %d  OK (ERROR_INIT)\n", res);
  }

  /* ================================================================
   *  Test 2 : mthread_cond_wait (cas d'erreur : mutex non verrouillé)
   * ================================================================ */
  {
    mthread_mutex_init(nullptr, &mutex);

    int res = mthread_cond_wait(&cond, &mutex);
    assert(res == MTHREAD_COND_ERROR_NOT_LOCKED);
    mthread_log("MAIN", "[wait]  cond_wait(mutex non lock) = %d  OK (ERROR_NOT_LOCKED)\n", res);
  }

  /* ================================================================
   *  Test 3 : mthread_cond_signal  (signal sans waiters → no-op)
   * ================================================================ */
  {
    int res = mthread_cond_signal(&cond);
    assert(res == 0);
    mthread_log("MAIN", "[signal] cond_signal (pas de waiters) = %d  OK (no-op)\n", res);

    res = mthread_cond_signal(nullptr);
    assert(res == MTHREAD_COND_ERROR_NULL);
    mthread_log("MAIN", "[signal] cond_signal(nullptr)         = %d  OK (ERROR_NULL)\n", res);
  }

  /* ================================================================
   *  Test 4 : mthread_cond_broadcast (broadcast sans waiters → no-op)
   * ================================================================ */
  {
    int res = mthread_cond_broadcast(&cond);
    assert(res == 0);
    mthread_log("MAIN", "[bcast] cond_broadcast (pas de waiters) = %d  OK (no-op)\n", res);

    res = mthread_cond_broadcast(nullptr);
    assert(res == MTHREAD_COND_ERROR_NULL);
    mthread_log("MAIN", "[bcast] cond_broadcast(nullptr)         = %d  OK (ERROR_NULL)\n", res);
  }

  /* ================================================================
   *  Test 5 : Scénario complet multi-thread
   *    3 workers wait → 1 signal (réveille 1) → 1 broadcast (réveille 2)
   * ================================================================ */
  {
    mthread_log("MAIN", "--- Lancement du scénario multi-thread ---\n");

    /* Lancer 3 workers */
    mthread_t tids[NB_WORKERS + 2];
    for (long i = 0; i < NB_WORKERS; i++) {
      tids[i] = mthread_create_thread(nullptr, worker, (void *)i);
      mthread_yield();
    }

    /* Lancer le signaler puis le broadcaster */
    tids[NB_WORKERS]     = mthread_create_thread(nullptr, signaler, nullptr);
    mthread_yield();
    tids[NB_WORKERS + 1] = mthread_create_thread(nullptr, broadcaster, nullptr);

    /* Joindre tous les threads */
    for (int i = 0; i < NB_WORKERS + 2; i++) {
      void *ret;
      mthread_join(tids[i], &ret);
      assert(ret == nullptr);
    }

    assert(woken == NB_WORKERS);
    mthread_log("MAIN", "[multi] woken=%d (attendu: %d) OK\n", woken, NB_WORKERS);
  }

  /* ================================================================
   *  Test 6 : mthread_cond_destroy
   * ================================================================ */
  {
    int res;

    /* destroy normal */
    res = mthread_cond_destroy(&cond);
    assert(res == 0);
    mthread_log("MAIN", "[destroy] cond_destroy(&cond)   = %d  OK\n", res);

    /* double destroy → erreur */
    res = mthread_cond_destroy(&cond);
    assert(res == MTHREAD_COND_ERROR_INIT);
    mthread_log("MAIN", "[destroy] cond_destroy x2       = %d  OK (ERROR_INIT)\n", res);

    /* destroy sur nullptr → erreur */
    res = mthread_cond_destroy(nullptr);
    assert(res == MTHREAD_COND_ERROR_NULL);
    mthread_log("MAIN", "[destroy] cond_destroy(nullptr) = %d  OK (ERROR_NULL)\n", res);
  }

  /* ---- Nettoyage ---- */
  mthread_mutex_destroy(&mutex);

  mthread_log("MAIN", "=== Tous les tests Q.6 OK ===\n");
  return 0;
}
