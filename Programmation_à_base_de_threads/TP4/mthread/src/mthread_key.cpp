#include <mthread.h>
#include <mthread_common_helpers.h>
#include <mthread_thread.h>
#include <mthread_vp_internal.h>
#include <cstring>

/* ============================================================
 *  Table globale des clés TLS
 *  Chaque entrée contient :
 *   - in_use      : la clé est-elle actuellement allouée ?
 *   - destructor  : fonction optionnelle appelée sur la valeur
 *                   quand un thread se termine (non implémenté
 *                   dans cette version simplifiée)
 * ============================================================ */

static struct {
  bool in_use;
  void (*destructor)(void *);
} key_table[MTHREAD_KEYS_MAX] = {};

static atomic_flag key_table_lock = ATOMIC_FLAG_INIT;

/* ============================================================
 *  Q.7 : mthread_key_create
 *
 *  Alloue une nouvelle clé TLS.
 *  - key       : pointeur vers la clé à initialiser
 *  - destructor: fonction optionnelle (peut être nullptr)
 * ============================================================ */
int mthread_key_create(mthread_key_t *key, void (*destructor)(void *)) {
  if (key == nullptr) {
    return MTHREAD_KEY_ERROR_NULL;
  }

  mthread_internal_spin_lock(&key_table_lock);

  /* Chercher le premier emplacement libre */
  for (unsigned int i = 0; i < MTHREAD_KEYS_MAX; i++) {
    if (!key_table[i].in_use) {
      key_table[i].in_use = true;
      key_table[i].destructor = destructor;
      *key = i;
      mthread_internal_spin_unlock(&key_table_lock);
      return 0;
    }
  }

  mthread_internal_spin_unlock(&key_table_lock);
  return MTHREAD_KEY_ERROR_MAX;  /* plus de clés disponibles */
}

/* ============================================================
 *  Q.8 : mthread_key_delete
 *
 *  Libère une clé TLS.  Ne libère PAS les données associées
 *  dans chaque thread (comportement POSIX).
 * ============================================================ */
int mthread_key_delete(mthread_key_t key) {
  if (key >= MTHREAD_KEYS_MAX) {
    return MTHREAD_KEY_ERROR_INVALID;
  }

  mthread_internal_spin_lock(&key_table_lock);

  if (!key_table[key].in_use) {
    mthread_internal_spin_unlock(&key_table_lock);
    return MTHREAD_KEY_ERROR_INVALID;
  }

  key_table[key].in_use = false;
  key_table[key].destructor = nullptr;

  mthread_internal_spin_unlock(&key_table_lock);
  return 0;
}

/* ============================================================
 *  Q.9 : mthread_setspecific
 *
 *  Associe une valeur au thread courant pour la clé donnée.
 * ============================================================ */
int mthread_setspecific(mthread_key_t key, const void *value) {
  if (key >= MTHREAD_KEYS_MAX) {
    return MTHREAD_KEY_ERROR_INVALID;
  }

  /* Vérifier que la clé est allouée (lecture seule, pas de lock nécessaire
     car une clé valide n'est pas supprimée pendant qu'on écrit) */
  if (!key_table[key].in_use) {
    return MTHREAD_KEY_ERROR_INVALID;
  }

  mthread_thread_t *self = mthread_self();
  self->tls_values[key] = (void *)value;
  return 0;
}

/* ============================================================
 *  Q.10 : mthread_getspecific
 *
 *  Retourne la valeur associée au thread courant pour la clé
 *  donnée, ou nullptr si la clé est invalide ou non initialisée.
 * ============================================================ */
void *mthread_getspecific(mthread_key_t key) {
  if (key >= MTHREAD_KEYS_MAX) {
    return nullptr;
  }

  if (!key_table[key].in_use) {
    return nullptr;
  }

  mthread_thread_t *self = mthread_self();
  return self->tls_values[key];
}
