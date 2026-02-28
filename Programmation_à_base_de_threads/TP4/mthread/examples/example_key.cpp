/**
 * Q.11 : Programme d'exemple testant les 4 fonctions de clés TLS
 *   - mthread_key_create   (Q.7)
 *   - mthread_key_delete   (Q.8)
 *   - mthread_setspecific  (Q.9)
 *   - mthread_getspecific  (Q.10)
 *
 * Scénario :
 *   On crée une clé TLS.  Chaque thread y stocke une valeur
 *   qui lui est propre (son identifiant cast en void*).
 *   On vérifie que chaque thread lit bien SA propre valeur
 *   et non celle d'un autre thread.
 */

#include <cassert>
#include <cstdio>
#include <mthread.h>

/* ---------- Clé TLS partagée ---------- */
static mthread_key_t my_key;

/* ---------- Thread worker ---------- */

void *worker(void *arg) {
  long id = (long)arg;

  /* Q.9 : stocker une valeur propre au thread */
  int res = mthread_setspecific(my_key, (void *)(id * 100 + 42));
  assert(res == 0);

  mthread_log("WORK", "Thread %ld : setspecific → %ld\n", id, id * 100 + 42);

  /* Faire un yield pour laisser d'autres threads modifier leur TLS */
  mthread_yield();

  /* Q.10 : relire la valeur — elle doit être celle de CE thread */
  void *val = mthread_getspecific(my_key);
  long expected = id * 100 + 42;
  mthread_log("WORK", "Thread %ld : getspecific → %ld (attendu %ld)\n",
              id, (long)val, expected);
  assert((long)val == expected);

  return nullptr;
}

/* ---------- Programme principal ---------- */

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  const int NB_VP = 2;
  const int NB_WORKERS = 5;

  mthread_init_scheduler_fifo_class(NB_VP);
  mthread_log("MAIN", "=== Q.11 : Test des clés TLS ===\n");

  /* ================================================================
   *  Test 1 : mthread_key_create — cas normaux et erreurs
   * ================================================================ */
  {
    int res;

    /* Création normale */
    res = mthread_key_create(&my_key, nullptr);
    assert(res == 0);
    mthread_log("MAIN", "[create] key_create(&key, nullptr) = %d, key=%u  OK\n",
                res, my_key);

    /* nullptr → erreur */
    res = mthread_key_create(nullptr, nullptr);
    assert(res == MTHREAD_KEY_ERROR_NULL);
    mthread_log("MAIN", "[create] key_create(nullptr)       = %d  OK (ERROR_NULL)\n", res);

    /* Créer plusieurs clés pour vérifier l'allocation séquentielle */
    mthread_key_t keys[3];
    for (int i = 0; i < 3; i++) {
      res = mthread_key_create(&keys[i], nullptr);
      assert(res == 0);
      mthread_log("MAIN", "[create] clé supplémentaire %d → key=%u  OK\n", i, keys[i]);
    }
    /* Nettoyer les clés supplémentaires */
    for (int i = 0; i < 3; i++) {
      mthread_key_delete(keys[i]);
    }
  }

  /* ================================================================
   *  Test 2 : mthread_setspecific / mthread_getspecific — cas d'erreur
   * ================================================================ */
  {
    int res;

    /* setspecific avec clé invalide */
    res = mthread_setspecific(9999, (void *)0xDEAD);
    assert(res == MTHREAD_KEY_ERROR_INVALID);
    mthread_log("MAIN", "[set]    setspecific(9999)   = %d  OK (ERROR_INVALID)\n", res);

    /* getspecific avec clé invalide → nullptr */
    void *val = mthread_getspecific(9999);
    assert(val == nullptr);
    mthread_log("MAIN", "[get]    getspecific(9999)   = %p  OK (nullptr)\n", val);

    /* setspecific / getspecific dans le main thread */
    res = mthread_setspecific(my_key, (void *)0xCAFE);
    assert(res == 0);
    val = mthread_getspecific(my_key);
    assert(val == (void *)0xCAFE);
    mthread_log("MAIN", "[set/get] main thread : set 0xCAFE, get %p  OK\n", val);
  }

  /* ================================================================
   *  Test 3 : Isolation entre threads (scénario multi-thread)
   *
   *  Chaque worker stocke une valeur différente dans la même clé.
   *  Après yield, chacun vérifie qu'il relit SA valeur.
   * ================================================================ */
  {
    mthread_log("MAIN", "--- Lancement de %d workers ---\n", NB_WORKERS);

    mthread_t tids[NB_WORKERS];
    for (long i = 0; i < NB_WORKERS; i++) {
      tids[i] = mthread_create_thread(nullptr, worker, (void *)i);
    }
    for (int i = 0; i < NB_WORKERS; i++) {
      void *ret;
      mthread_join(tids[i], &ret);
      assert(ret == nullptr);
    }

    mthread_log("MAIN", "[multi] Tous les workers ont lu leur propre valeur  OK\n");
  }

  /* ================================================================
   *  Test 4 : mthread_key_delete
   * ================================================================ */
  {
    int res;

    /* delete normal */
    res = mthread_key_delete(my_key);
    assert(res == 0);
    mthread_log("MAIN", "[delete] key_delete(key)      = %d  OK\n", res);

    /* double delete → erreur */
    res = mthread_key_delete(my_key);
    assert(res == MTHREAD_KEY_ERROR_INVALID);
    mthread_log("MAIN", "[delete] key_delete x2        = %d  OK (ERROR_INVALID)\n", res);

    /* delete clé hors limites */
    res = mthread_key_delete(9999);
    assert(res == MTHREAD_KEY_ERROR_INVALID);
    mthread_log("MAIN", "[delete] key_delete(9999)     = %d  OK (ERROR_INVALID)\n", res);

    /* getspecific après delete → nullptr */
    void *val = mthread_getspecific(my_key);
    assert(val == nullptr);
    mthread_log("MAIN", "[get]    getspecific après delete = %p  OK (nullptr)\n", val);
  }

  mthread_log("MAIN", "=== Tous les tests Q.11 OK ===\n");
  return 0;
}
