# TP 3 — Sémaphores dans mthread

---

## I. Sémaphore — Exemple

### Q.1 : Décrire à l'aide d'un schéma l'utilité du sémaphore dans l'exemple précédent

Dans l'exemple `q1.c`, deux threads (`filsA` et `filsB`) exécutent la même fonction `affichage()` qui imprime des caractères ("A" ou "B") sur la sortie standard. Sans sémaphore, les affichages des deux threads s'entremêleraient de manière imprévisible (ex : `AAABBBAABB...`).

Le sémaphore `mutex` (initialisé à 1) est utilisé comme **verrou d'exclusion mutuelle** :

```
Thread A                          Thread B
   |                                 |
   |--- sem_wait(&mutex) ------->   |  (value: 1 → 0, A entre)
   |    [affiche AAAAA]             |
   |    sched_yield()               |--- sem_wait(&mutex)  (value=0, B BLOQUÉ)
   |    [affiche AAAAA + \n]        |         |
   |--- sem_post(&mutex) ------->   |         | (value: 0 → réveille B)
   |                                |    [affiche BBBBB]
   |--- sem_wait(&mutex)            |    sched_yield()
   |   (value=0, A BLOQUÉ)         |    [affiche BBBBB + \n]
   |         |                      |--- sem_post(&mutex)  (réveille A)
   |         |                      |
   ...      ...                    ...
```

**Utilité** : Le sémaphore garantit que chaque ligne complète (10 caractères identiques + `\n`) est affichée **de manière atomique**, sans entrelacement. Même si `sched_yield()` est appelé au milieu de l'affichage, le thread B ne peut pas intervenir car il est bloqué sur `sem_wait`. Cela assure que la sortie sera toujours des lignes de `AAAAAAAAAA` ou `BBBBBBBBBB`, jamais mélangées.

---

## II. Mise en place des sémaphores dans mthread

### Structure du sémaphore

La structure `mthread_sem_t` a été définie dans `mthread.h` :

```c
typedef struct mthread_sem_s {
  bool is_initialized = false;        // Indique si le sémaphore est initialisé
  atomic_int value = 0;               // Compteur atomique du sémaphore
  atomic_flag thread_list_spinlock = ATOMIC_FLAG_INIT;  // Spinlock pour protéger la liste
  mthread_list_t thread_list;         // Liste des threads en attente
} mthread_sem_t;
```

---

### Q.2 : `mthread_sem_init`

**Rôle** : Initialise un sémaphore avec une valeur donnée.

**Implémentation** :

```cpp
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
  sem->thread_list_spinlock = (atomic_flag)ATOMIC_FLAG_INIT;
  sem->thread_list.head = nullptr;
  sem->thread_list.tail = nullptr;
  return 0;
}
```

**Détails** :
- On vérifie que le pointeur `sem` n'est pas `NULL`.
- On refuse les valeurs négatives (`MTHREAD_SEM_ERROR_INIT`).
- On refuse une double initialisation (`MTHREAD_SEM_ERROR_INIT`).
- On stocke la valeur de manière atomique et on initialise le spinlock et la liste d'attente à vide.

---

### Q.3 : `mthread_sem_wait`

**Rôle** : Décrémente le sémaphore. Si la valeur est 0, le thread appelant est bloqué jusqu'à ce qu'un autre thread fasse `sem_post`.

**Implémentation** :

```cpp
int mthread_sem_wait(mthread_sem_t *sem) {
  if (sem == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (!sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }

  mthread_internal_spin_lock(&(sem->thread_list_spinlock));

  if (atomic_load(&(sem->value)) > 0) {
    atomic_fetch_sub(&(sem->value), 1);
    mthread_internal_spin_unlock(&(sem->thread_list_spinlock));
    return 0;
  }

  // Valeur == 0 : on bloque le thread
  mthread_thread_t *current_thread = mthread_self();
  mthread_list_item_t item;
  item.thread = current_thread;
  insert_tail_in_list(&item, &sem->thread_list);
  mthread_block_self(&(sem->thread_list_spinlock));

  return 0;
}
```

**Détails** :
- On prend le spinlock pour protéger l'accès concurrent à `value` et à la liste d'attente.
- Si `value > 0` : on décrémente et on libère le spinlock → le thread continue.
- Si `value == 0` : on insère le thread courant dans la liste d'attente (`thread_list`) puis on appelle `mthread_block_self` qui bloque le thread **et** libère le spinlock de manière atomique (même mécanisme que le mutex).

---

### Q.4 : `mthread_sem_post`

**Rôle** : Incrémente le sémaphore. Si des threads sont en attente, en réveille un.

**Implémentation** :

```cpp
int mthread_sem_post(mthread_sem_t *sem) {
  if (sem == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (!sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }

  mthread_internal_spin_lock(&(sem->thread_list_spinlock));

  if (sem->thread_list.head != nullptr) {
    mthread_list_item_t *item = remove_head_in_list(&(sem->thread_list));
    mthread_internal_spin_unlock(&(sem->thread_list_spinlock));
    mthread_wake_thread(item->thread);
  } else {
    atomic_fetch_add(&(sem->value), 1);
    mthread_internal_spin_unlock(&(sem->thread_list_spinlock));
  }

  return 0;
}
```

**Détails** :
- On prend le spinlock.
- S'il y a des threads bloqués dans `thread_list` : on retire le premier de la liste (FIFO) et on le réveille avec `mthread_wake_thread`. La valeur du sémaphore ne change pas (le jeton est transféré directement au thread réveillé).
- S'il n'y a pas de thread en attente : on incrémente simplement la valeur.

---

### Q.5 : `mthread_sem_destroy`

**Note** : La fonction `mthread_sem_destroy` n'est pas déclarée dans l'API `mthread.h` de ce TD. Néanmoins, voici comment elle pourrait être implémentée par analogie avec `mthread_mutex_destroy` :

```cpp
int mthread_sem_destroy(mthread_sem_t *sem) {
  if (sem == nullptr) {
    return MTHREAD_SEM_ERROR_NULL;
  }
  if (!sem->is_initialized) {
    return MTHREAD_SEM_ERROR_INIT;
  }
  // On ne peut pas détruire un sémaphore si des threads sont en attente
  if (sem->thread_list.head != nullptr) {
    return MTHREAD_SEM_ERROR_ALREADY_LOCKED;
  }
  sem->is_initialized = false;
  atomic_store(&(sem->value), 0);
  sem->thread_list.head = nullptr;
  sem->thread_list.tail = nullptr;
  return 0;
}
```

**Détails** :
- On vérifie que le sémaphore est initialisé.
- On refuse la destruction si des threads sont encore bloqués dans la file d'attente.
- On remet tous les champs à leur état initial.

---

### Q.6 : `mthread_sem_trywait`

**Rôle** : Tente de décrémenter le sémaphore de manière **non bloquante**. Si la valeur est 0, retourne une erreur au lieu de bloquer.

**Implémentation** :

```cpp
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
```

**Détails** :
- On lit la valeur courante du sémaphore.
- Si `value > 0` : on décrémente et on retourne 0 (succès).
- Si `value == 0` : on retourne `MTHREAD_SEM_ERROR_ALREADY_LOCKED` sans bloquer le thread. C'est la différence fondamentale avec `sem_wait`.

---

### Q.7 : `mthread_sem_getvalue`

**Rôle** : Récupère la valeur courante du sémaphore.

**Implémentation** :

```cpp
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
```

**Détails** :
- On vérifie que ni `sem` ni `val` ne sont `NULL`.
- On vérifie que le sémaphore est initialisé.
- On lit la valeur atomiquement et on l'écrit dans `*val`.

---

## III. Démonstration

### Q.8 : Programmes d'exemple testant chaque fonction

#### Test de `mthread_sem_init`

```cpp
// Vérifie : init normal, init avec valeur négative, double init
mthread_sem_t sem;
int res;

res = mthread_sem_init(&sem, 1);    // Doit retourner 0
assert(res == 0);

mthread_sem_t sem2;
res = mthread_sem_init(&sem2, -1);  // Doit retourner MTHREAD_SEM_ERROR_INIT
assert(res == MTHREAD_SEM_ERROR_INIT);

res = mthread_sem_init(&sem, 1);    // Double init → MTHREAD_SEM_ERROR_INIT
assert(res == MTHREAD_SEM_ERROR_INIT);
```

#### Test de `mthread_sem_wait`

```cpp
// Vérifie : wait décrémente la valeur
mthread_sem_t sem;
int res, val;

mthread_sem_init(&sem, 2);
mthread_sem_wait(&sem);
mthread_sem_getvalue(&sem, &val);
assert(val == 1);  // 2 - 1 = 1

mthread_sem_wait(&sem);
mthread_sem_getvalue(&sem, &val);
assert(val == 0);  // 1 - 1 = 0
```

#### Test de `mthread_sem_post`

```cpp
// Vérifie : post incrémente la valeur
mthread_sem_t sem;
int res, val;

mthread_sem_init(&sem, 0);
mthread_sem_post(&sem);
mthread_sem_getvalue(&sem, &val);
assert(val == 1);  // 0 + 1 = 1

mthread_sem_post(&sem);
mthread_sem_getvalue(&sem, &val);
assert(val == 2);  // 1 + 1 = 2
```

#### Test de `mthread_sem_trywait`

```cpp
// Vérifie : trylock non bloquant
mthread_sem_t sem;
int res;

mthread_sem_init(&sem, 1);
res = mthread_sem_trylock(&sem);
assert(res == 0);  // Succès, value passe de 1 à 0

res = mthread_sem_trylock(&sem);
assert(res == MTHREAD_SEM_ERROR_ALREADY_LOCKED);  // Échec, value == 0

mthread_sem_post(&sem);  // value remonte à 1
res = mthread_sem_trylock(&sem);
assert(res == 0);  // Succès à nouveau
```

#### Test de `mthread_sem_getvalue`

```cpp
// Vérifie : lecture de la valeur et gestion des erreurs
mthread_sem_t sem;
int res, val;

mthread_sem_init(&sem, 42);
res = mthread_sem_getvalue(&sem, &val);
assert(res == 0 && val == 42);

res = mthread_sem_getvalue(nullptr, &val);
assert(res == MTHREAD_SEM_ERROR_NULL);  // sem NULL

res = mthread_sem_getvalue(&sem, nullptr);
assert(res == MTHREAD_SEM_ERROR_NULL);  // val NULL
```

#### Test multi-thread (wait bloquant + post)

```cpp
// Vérifie le comportement bloquant de sem_wait en multi-thread
mthread_sem_t sem;

void* thread_post(void* arg) {
    mthread_sem_post(&sem);
    return nullptr;
}

void* thread_wait(void* arg) {
    mthread_sem_wait(&sem);  // Bloque si value == 0
    mthread_sem_wait(&sem);  // Attend le post
    return nullptr;
}

int main() {
    mthread_sem_init(&sem, 1);
    // Valeur initiale = 1
    // thread_post fait +1 → value = 2
    // thread_wait fait -1, -1 → value = 0
    mthread_t t1 = mthread_create_thread(nullptr, thread_post, nullptr);
    mthread_t t2 = mthread_create_thread(nullptr, thread_wait, nullptr);
    mthread_join(t1, nullptr);
    mthread_join(t2, nullptr);

    int val;
    mthread_sem_getvalue(&sem, &val);
    assert(val == 0);  // 1 + 1 - 1 - 1 = 0
}
```
