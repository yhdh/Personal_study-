# Notes de Programmation Parallèle - S4

---

# 1. OpenMP (Open Multi-Processing)

## Introduction

**OpenMP** est une API de programmation parallèle pour systèmes à mémoire partagée. Elle permet de paralléliser facilement du code séquentiel à l'aide de **directives** de compilation.

## Historique

- **PFC** (Parallel Computing Forum) : à l'origine des standards
- **Machines vectorielles** : CRAY, NEC, IBM
- **Standards principaux** : MPI (mémoire distribuée) et OpenMP (mémoire partagée)

## Concepts clés

| Concept | Description |
|---------|-------------|
| **Team** | Ensemble de threads participant à l'exécution d'une tâche |
| **Région parallèle** | Section de code exécutée par plusieurs threads |
| **Région séquentielle** | Section exécutée par un seul thread (master) |
| **Directive** | Instruction au compilateur pour paralléliser |
| **Clause** | Option qui modifie le comportement d'une directive |

## Processeur Xeon Phi (Intel)

- 72 cœurs
- Vecteurs longs (optimisé pour SIMD)
- Moins puissant par cœur, mais massivement parallèle

## Modèle de programmation

1. Écrire un **code séquentiel** fonctionnel
2. Identifier les **régions parallélisables**
3. Ajouter les **directives OpenMP**
4. Le compilateur génère le code parallèle

## Compilation

```bash
gcc -fopenmp programme.c -o programme
```

## Limitations

- Dépendances de données non toujours détectées par le compilateur
- Vectorisation automatique limitée
- Dépendances entre boucles à gérer manuellement

## Exemple basique

```c
#include <omp.h>
#include <stdio.h>

int main() {
    #pragma omp parallel
    {
        printf("Thread %d sur %d\n", omp_get_thread_num(), omp_get_num_threads());
    }
    return 0;
}
```

## À faire

- [ ] Faire des exercices sur la programmation parallèle
- [ ] Apprendre Fortran (utilisé en calcul scientifique)

---

# 2. Programmation à base de Threads (POSIX pthread)

## Introduction

**pthread** (POSIX Threads) est l'API standard pour la programmation multi-thread sur systèmes Unix/Linux/macOS.

## Inclusion et compilation

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
```

```bash
gcc -pthread programme.c -o programme
```

> ⚠️ C'est `pthread.h` (sans 's'), pas `pthreads.h`

## Fonctions principales

| Fonction | Description |
|----------|-------------|
| `pthread_create()` | Crée un nouveau thread |
| `pthread_join()` | Attend la fin d'un thread (bloquant) |
| `pthread_self()` | Retourne l'ID du thread courant |
| `pthread_exit()` | Termine le thread courant |
| `pthread_cancel()` | Annule un thread |
| `pthread_detach()` | Détache un thread (libération auto des ressources) |

---

## pthread_create

```c
int pthread_create(
    pthread_t *thread,              // 1. Pointeur vers l'ID du thread créé
    const pthread_attr_t *attr,     // 2. Attributs (NULL = défaut)
    void *(*start_routine)(void *), // 3. Fonction à exécuter
    void *arg                       // 4. Argument passé à la fonction
);
```

**Retour :** `0` en cas de succès, code d'erreur sinon.

**Exemple minimal :**
```c
#include <pthread.h>
#include <stdio.h>

void *ma_fonction(void *arg) {
    printf("Hello depuis le thread !\n");
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, ma_fonction, NULL);
    pthread_join(thread, NULL);
    return 0;
}
```

---

## pthread_join

```c
int pthread_join(pthread_t thread, void **retval);
```

- **Bloque** le thread appelant jusqu'à la fin du thread cible
- `retval` : récupère la valeur retournée par le thread (ou `NULL`)

**Pattern classique :**
```c
pthread_t threads[N];

// Créer tous les threads
for (int i = 0; i < N; i++) {
    pthread_create(&threads[i], NULL, fonction, NULL);
}

// Attendre tous les threads
for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
}
```

> ⚠️ Ne pas mettre `pthread_join` dans la boucle de création, sinon exécution séquentielle !

---

## Passer des arguments aux threads

### Méthode 1 : Cast direct (entier → pointeur)

```c
// Envoi (dans main)
for (long i = 0; i < n; i++) {
    pthread_create(&threads[i], NULL, fonction, (void *)i);
}

// Réception (dans le thread)
void *fonction(void *arg) {
    long num = (long)arg;
    printf("Je suis le thread %ld\n", num);
    return NULL;
}
```

> ⚠️ Fonctionne si `sizeof(long) >= sizeof(void *)`

### Méthode 2 : Utiliser une structure (recommandé)

```c
typedef struct {
    int id;
    char *message;
} thread_data;

void *fonction(void *arg) {
    thread_data *data = (thread_data *)arg;
    printf("Thread %d : %s\n", data->id, data->message);
    return NULL;
}

int main() {
    pthread_t threads[N];
    thread_data data[N];
    
    for (int i = 0; i < N; i++) {
        data[i].id = i;
        data[i].message = "Hello";
        pthread_create(&threads[i], NULL, fonction, &data[i]);
    }
    
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
```

### Méthode 3 : Allocation dynamique

```c
for (int i = 0; i < N; i++) {
    thread_data *data = malloc(sizeof(thread_data));
    data->id = i;
    sprintf(data->message, "Hello thread %d", i);
    pthread_create(&threads[i], NULL, fonction, data);
}
```

> ⚠️ Ne pas oublier de `free()` dans le thread !

---

## ID d'un thread (pthread_t)

- **Identifiant unique** attribué à chaque thread lors de sa création
- Type opaque (peut être entier ou structure selon l'OS)

```c
pthread_t id;
pthread_create(&id, NULL, fonction, NULL);

// Afficher l'ID
printf("ID (décimal): %lu\n", (unsigned long)id);
printf("ID (hexa): %p\n", (void *)id);

// Obtenir l'ID du thread courant
pthread_t self = pthread_self();
```

---

## Attributs d'un thread (pthread_attr_t)

Le 2ème argument de `pthread_create` permet de configurer le thread.

| Attribut | Fonction | Valeurs |
|----------|----------|---------|
| `detachstate` | Joignable ou détaché | `PTHREAD_CREATE_JOINABLE` (défaut) / `PTHREAD_CREATE_DETACHED` |
| `stacksize` | Taille de la pile | En octets (~8 Mo par défaut) |
| `schedpolicy` | Politique d'ordonnancement | `SCHED_FIFO`, `SCHED_RR`, `SCHED_OTHER` |
| `priority` | Priorité | Dépend de la politique |

**Exemple :**
```c
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_attr_setstacksize(&attr, 1024 * 1024);  // 1 Mo

pthread_create(&thread, &attr, fonction, NULL);
pthread_attr_destroy(&attr);
```

**Thread détaché :** Les ressources sont libérées automatiquement à la fin (pas besoin de `pthread_join`).

---

## Non-déterminisme de l'ordonnancement

L'ordre d'exécution des threads **n'est pas garanti**.

```
Exécution 1:          Exécution 2:
Thread 0: Hello       Thread 2: Hello
Thread 1: Hello       Thread 0: Hello
Thread 2: Hello       Thread 1: Hello
```

**Raison :** L'ordonnanceur (scheduler) de l'OS décide quel thread s'exécute à chaque instant selon :
- La charge CPU
- Les priorités
- Les interruptions

---

## Race condition (condition de concurrence)

### Le problème

Plusieurs threads accèdent **simultanément** à une variable partagée.

```c
unsigned long count = 0;  // Variable partagée

void *parallel_count(void *args) {
    for (int i = 0; i < 10000; ++i) {
        ++count;  // ⚠️ NON ATOMIQUE !
    }
    return NULL;
}
```

### Pourquoi `++count` n'est pas atomique ?

C'est en réalité **3 opérations** :
1. **Lire** la valeur de `count` depuis la mémoire
2. **Incrémenter** de 1
3. **Écrire** le résultat en mémoire

### Scénario problématique

```
Thread 1: lit count = 5
Thread 2: lit count = 5      ← Les deux lisent la même valeur !
Thread 1: écrit count = 6
Thread 2: écrit count = 6    ← Devrait être 7 !
```

**Résultat attendu :** 10 threads × 10000 = **100000**
**Résultat obtenu :** < 100000 (et variable à chaque exécution)

### Solution : Mutex (Mutual Exclusion)

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *parallel_count(void *args) {
    for (int i = 0; i < 10000; ++i) {
        pthread_mutex_lock(&lock);    // Verrouiller
        ++count;                       // Section critique
        pthread_mutex_unlock(&lock);  // Déverrouiller
    }
    return NULL;
}
```

| Fonction | Description |
|----------|-------------|
| `pthread_mutex_init()` | Initialise un mutex |
| `pthread_mutex_lock()` | Verrouille (bloque si déjà pris) |
| `pthread_mutex_unlock()` | Déverrouille |
| `pthread_mutex_destroy()` | Détruit le mutex |

---

## Fonctions utiles en C

### sprintf vs printf vs fprintf

| Fonction | Destination | Exemple |
|----------|-------------|---------|
| `printf` | Écran (stdout) | `printf("Hello %d", x);` |
| `sprintf` | Buffer (chaîne) | `sprintf(buf, "Hello %d", x);` |
| `fprintf` | Fichier | `fprintf(file, "Hello %d", x);` |

```c
char buf[100];
sprintf(buf, "Thread %d prêt\n", id);  // Écrit dans buf
printf("%s", buf);                      // Affiche buf
```

### Formats printf

| Type | Format | Exemple |
|------|--------|---------|
| `int` | `%d` ou `%i` | `printf("%d", 42);` |
| `long` | `%ld` ou `%li` | `printf("%ld", 42L);` |
| `unsigned long` | `%lu` | `printf("%lu", 42UL);` |
| `float` / `double` | `%f` | `printf("%f", 3.14);` |
| `char` | `%c` | `printf("%c", 'A');` |
| `char *` (string) | `%s` | `printf("%s", "Hello");` |
| `pointeur` | `%p` | `printf("%p", ptr);` |

### Arguments du programme (argc, argv)

```c
#include <stdlib.h>

int main(int argc, char **argv) {
    // argc : nombre d'arguments (incluant le nom du programme)
    // argv[0] : nom du programme
    // argv[1] : premier argument
    // argv[2] : deuxième argument, etc.
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <nombre>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);  // Convertit string → int
    printf("Argument reçu : %d\n", n);
    
    return 0;
}
```

```bash
./programme 42
# argc = 2
# argv[0] = "./programme"
# argv[1] = "42"
```

### Allocation dynamique

```c
#include <stdlib.h>

// Allouer
int *ptr = (int *)malloc(sizeof(int));
int *tab = (int *)malloc(n * sizeof(int));

// Utiliser
*ptr = 42;
tab[0] = 1;

// Libérer (important !)
free(ptr);
free(tab);
```

---

## Résumé : Squelette d'un programme multi-thread

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

void *thread_function(void *arg) {
    long id = (long)arg;
    printf("Thread %ld en cours d'exécution\n", id);
    // ... travail du thread ...
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t threads[NUM_THREADS];
    
    // Création des threads
    for (long i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_function, (void *)i) != 0) {
            perror("Erreur pthread_create");
            return 1;
        }
    }
    
    // Attente de tous les threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Tous les threads sont terminés\n");
    return 0;
}
```

---

## Erreurs courantes

| Erreur | Problème | Solution |
|--------|----------|----------|
| `pthread_join` dans la boucle de création | Exécution séquentielle | Deux boucles séparées |
| Oublier `-pthread` à la compilation | Erreurs de liaison | Ajouter le flag |
| Passer `&i` au lieu de `(void *)i` | Tous les threads voient la même valeur | Cast direct ou allocation |
| Ne pas protéger les variables partagées | Race condition | Utiliser un mutex |
| Oublier `pthread_join` | Programme termine avant les threads | Toujours joindre ou détacher | 