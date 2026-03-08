# TP4 — Programmation OpenMP : Tâches

## Table des matières

1. [Partie I — Parcours d'un arbre (Depth First Search)](#partie-i--parcours-dun-arbre-depth-first-search)
   - [Q.1 — Version séquentielle](#q1--version-séquentielle)
   - [Q.2 — Parallélisation avec tâches OpenMP](#q2--parallélisation-avec-tâches-openmp)
   - [Q.3 — Limitation de la profondeur de génération de tâches](#q3--limitation-de-la-profondeur-de-génération-de-tâches)
   - [Q.4 — Analyse des performances](#q4--analyse-des-performances)
   - [Q.5 — `taskgroup` vs `taskwait`](#q5--taskgroup-vs-taskwait)
2. [Partie II — Stencil](#partie-ii--stencil)
   - [Q.6 — Compilation et exécution](#q6--compilation-et-exécution)
   - [Q.7 — Parallélisation avec tâches OpenMP](#q7--parallélisation-avec-tâches-openmp)

---

## Partie I — Parcours d'un arbre (Depth First Search)

**Fichier** : `Tree-traversal/tree-traverse.c`

Le programme construit un arbre binaire de recherche (BST) aléatoire et calcule sa profondeur via un algorithme DFS (Depth First Search). Toutes les versions sont compilées et exécutées dans un même binaire pour faciliter la comparaison.

---

### Q.1 — Version séquentielle

**Objectif** : Implémenter `inorderTraverse_Seq(struct node *r)` qui calcule la profondeur d'un arbre en séquentiel.

**Principe** : On parcourt récursivement les sous-arbres gauche et droit. Chaque appel retourne la profondeur du sous-arbre. On renvoie `1 + max(profondeur_gauche, profondeur_droite)` pour compter le niveau courant.

```c
int inorderTraverse_Seq(struct node *r)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    depthRight = inorderTraverse_Seq(r->right);
  }
  if (r->left != NULL)
  {
    depthLeft = inorderTraverse_Seq(r->left);
  }

  /* +1 pour compter le niveau courant */
  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}
```

**Explications** :
- **Cas de base** : si un nœud est une feuille (pas d'enfants), `depthLeft` et `depthRight` restent à 0, et la fonction retourne 1 (la feuille elle-même).
- **Cas récursif** : on descend dans chaque sous-arbre non-nul, puis on prend le maximum des deux profondeurs + 1.

---

### Q.2 — Parallélisation avec tâches OpenMP

**Objectif** : Paralléliser le parcours DFS avec `#pragma omp task`.

**Principe** : Chaque appel récursif est encapsulé dans une tâche OpenMP. Les deux sous-arbres (gauche et droit) peuvent être explorés en parallèle par des threads différents. Un `#pragma omp taskwait` synchronise les résultats avant de calculer le maximum.

```c
int inorderTraverse_Task(struct node *r)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    #pragma omp task shared(depthRight)
    {
      depthRight = inorderTraverse_Task(r->right);
    }
  }
  if (r->left != NULL)
  {
    #pragma omp task shared(depthLeft)
    {
      depthLeft = inorderTraverse_Task(r->left);
    }
  }

  #pragma omp taskwait

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}
```

**Points clés de l'implémentation** :
- **`shared(depthRight)` / `shared(depthLeft)`** : les variables de résultat doivent être partagées pour que la tâche puisse écrire le résultat et que le thread parent puisse le lire.
- **`#pragma omp taskwait`** : point de synchronisation indispensable — on doit attendre que les deux tâches enfants aient terminé avant de comparer les profondeurs.
- **Lancement** : une région `parallel` avec `single` est utilisée dans le `main` pour que seul un thread crée la première tâche, puis les tâches se répartissent naturellement entre les threads.

```c
#pragma omp parallel num_threads(numThreads)
{
  #pragma omp single
  {
    depth = inorderTraverse_Task(root);
  }
}
```

---

### Q.3 — Limitation de la profondeur de génération de tâches

**Problème** : La version Q.2 crée une tâche par nœud de l'arbre. Pour 1 million de nœuds, cela représente ~2 millions de tâches, ce qui génère un surcoût considérable (création, ordonnancement, synchronisation).

**Solution** : Limiter la création de tâches aux `MAX_DEPTH` premiers niveaux de récursion ($2^5 = 32$ tâches maximum). Au-delà, le parcours redevient séquentiel.

#### Méthode 1 — Clause `if`

```c
#define MAX_DEPTH 5

int inorderTraverse_Task_DepthLimit_If(struct node *r, int currentDepth)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    #pragma omp task shared(depthRight) if(currentDepth < MAX_DEPTH)
    {
      depthRight = inorderTraverse_Task_DepthLimit_If(r->right, currentDepth + 1);
    }
  }
  if (r->left != NULL)
  {
    #pragma omp task shared(depthLeft) if(currentDepth < MAX_DEPTH)
    {
      depthLeft = inorderTraverse_Task_DepthLimit_If(r->left, currentDepth + 1);
    }
  }

  #pragma omp taskwait

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}
```

**Fonctionnement de `if(condition)`** :
- Si `condition` est **vraie** : une tâche est créée normalement (peut être exécutée par un autre thread).
- Si `condition` est **fausse** (`if(0)`) : **aucune tâche n'est créée**. Le code du bloc est exécuté immédiatement par le thread courant, comme un appel de fonction classique.

#### Méthode 2 — Clause `final`

```c
int inorderTraverse_Task_DepthLimit_Final(struct node *r, int currentDepth)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    #pragma omp task shared(depthRight) final(currentDepth >= MAX_DEPTH)
    {
      depthRight = inorderTraverse_Task_DepthLimit_Final(r->right, currentDepth + 1);
    }
  }
  if (r->left != NULL)
  {
    #pragma omp task shared(depthLeft) final(currentDepth >= MAX_DEPTH)
    {
      depthLeft = inorderTraverse_Task_DepthLimit_Final(r->left, currentDepth + 1);
    }
  }

  #pragma omp taskwait

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}
```

**Fonctionnement de `final(condition)`** :
- Si `condition` est **vraie** : la tâche est créée mais marquée **finale**. Elle et toutes ses tâches descendantes seront exécutées **immédiatement** (séquentiellement), comme si elles étaient `if(0)`.
- **Différence avec `if`** : avec `final`, **un objet tâche est toujours créé** (même s'il s'exécute immédiatement), ce qui engendre un léger surcoût par rapport à `if(0)` qui ne crée aucune tâche.

---

### Q.4 — Analyse des performances

#### Résultats expérimentaux

**Configuration** : 1 000 000 nœuds, 4 threads

| Version | Temps (sec) | Speedup vs séq. |
|---|---|---|
| Q.1 — Séquentielle | **0.024** | 1.0x |
| Q.2 — Tasks (sans limite) | 0.295 | 0.08x (12x plus lent !) |
| Q.3a — `if` (limit=5) | **0.059** | 0.41x |
| Q.3b — `final` (limit=5) | 0.144 | 0.17x |
| Q.5 — `taskgroup` | 0.312 | 0.08x |

#### Analyse

**1. La version tasks sans limite est bien plus lente que la version séquentielle.**

Le surcoût de création/ordonnancement de ~2 millions de tâches est **largement supérieur** au travail utile de chaque tâche (une simple comparaison). Le grain de calcul par tâche est extrêmement fin : chaque tâche ne fait qu'une addition et une comparaison. Le rapport travail/surcoût est catastrophique.

**2. La clause `if` offre la meilleure performance parallèle.**

En limitant à 5 niveaux, on ne crée que $2^5 = 32$ tâches au maximum. Chaque tâche parcourt ensuite un sous-arbre complet séquentiellement. Le surcoût est minimal et on répartit efficacement le travail sur les 4 threads.

**3. La clause `final` est moins performante que `if`.**

Avec `final`, des objets tâches sont toujours créés à chaque nœud (même si elles s'exécutent immédiatement). Cela génère un surcoût de gestion mémoire absent avec `if(0)`.

**4. Le séquentiel reste le plus rapide ici.**

Pour ce type de problème à grain très fin (le travail par nœud est trivial), le parallélisme de tâches n'apporte pas de gain. Le parcours d'arbre est principalement limité par les accès mémoire (pointeurs) et le surcoût de synchronisation dépasse le gain potentiel. Un arbre BST aléatoire est aussi naturellement déséquilibré, ce qui limite la répartition de charge.

---

### Q.5 — `taskgroup` vs `taskwait`

```c
int inorderTraverse_Taskgroup(struct node *r)
{
  int depthLeft = 0;
  int depthRight = 0;

  #pragma omp taskgroup
  {
    if (r->right != NULL)
    {
      #pragma omp task shared(depthRight)
      {
        depthRight = inorderTraverse_Taskgroup(r->right);
      }
    }
    if (r->left != NULL)
    {
      #pragma omp task shared(depthLeft)
      {
        depthLeft = inorderTraverse_Taskgroup(r->left);
      }
    }
  } /* Fin du taskgroup : toutes les tâches descendantes sont terminées */

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}
```

#### Différence entre `taskwait` et `taskgroup`

| | `taskwait` | `taskgroup` |
|---|---|---|
| **Portée** | Attend les tâches enfants **directes** uniquement (1 niveau) | Attend **toutes** les tâches descendantes créées dans le bloc (enfants, petits-enfants, etc.) |
| **Syntaxe** | Directive simple (pas de bloc) | Directive structurée (avec un bloc `{ }`) |
| **Granularité** | Fine : ne synchronise qu'un niveau | Large : synchronise tout le sous-arbre de tâches |
| **Usage typique** | Quand chaque niveau gère sa propre synchronisation | Quand on veut garantir la complétion de tout un sous-graphe de tâches |

**Dans notre cas**, les deux fonctionnent correctement car chaque niveau de récursion fait son propre `taskwait`/`taskgroup`. Cependant, `taskgroup` est plus **robuste** : même si on oubliait un `taskwait` à un niveau intermédiaire, le `taskgroup` au niveau supérieur garantirait l'attente de toutes les tâches descendantes.

**En termes de performance**, `taskgroup` a un surcoût légèrement supérieur car il doit traquer toutes les tâches descendantes (pas seulement les enfants directs).

---

## Partie II — Stencil

**Fichier de base** : `STENCIL/stencil_seq.c`

Le programme crée un maillage 1D rempli de valeurs aléatoires et applique itérativement :
1. **Un flou** (blur) sur les extrémités gauche et droite du maillage (moyennage avec les voisins).
2. **Une copie** de la partie centrale (pas de blur).
3. **Une mise à l'échelle** (scale) sur tout le maillage.

La boucle s'arrête quand le blur converge (toutes les différences < `threshold`).

---

### Q.6 — Compilation et exécution

```bash
$ make
gcc -O3  -o stencil_seq stencil_seq.c
gcc -O3 -fopenmp -o stencil_task stencil_task.c
gcc -O3 -fopenmp -o stencil_taskdep stencil_taskdep.c

$ ./stencil_seq 100 10
Running stencil on 100 element(s) with seed 10
DONE w/ 5 iter in 0.000001 s
    MESH 0.00 0.02 0.04 0.05 0.05 0.06 0.06 0.06 0.07 0.08
```

Le Makefile compile 3 exécutables :
- `stencil_seq` : version séquentielle (fournie)
- `stencil_task` : version avec tâches + `taskwait`
- `stencil_taskdep` : version avec tâches + `depend`

---

### Q.7 — Parallélisation avec tâches OpenMP

#### Analyse des dépendances

Avant de paralléliser, identifions les dépendances entre les 4 boucles :

```
Itération k :
  Loop 1 (blur gauche)  : lit mesh[] → écrit mesh_blur[1..N/10-2]
  Loop 2 (copie milieu) : lit mesh[] → écrit mesh_blur[N/10-1..0.9N-2]
  Loop 3 (blur droit)   : lit mesh[] → écrit mesh_blur[0.9N-1..N-2]
  Loop 4 (scale)        : lit mesh_blur[] → écrit mesh[]
```

**Constat** :
- **Loops 1, 2, 3** travaillent sur des régions **disjointes** de `mesh_blur` et lisent toutes `mesh` → **parallèles entre elles**.
- **Loop 4** lit `mesh_blur` et écrit `mesh` → **dépend des loops 1, 2, 3**.
- **Itération k+1** lit `mesh` → **dépend de loop 4 de l'itération k**.

```
          ┌──── Loop 1 (blur gauche) ────┐
          │                              │
mesh[] ──>├──── Loop 2 (copie milieu) ───├──> mesh_blur[] ──> Loop 4 (scale) ──> mesh[]
          │                              │                                         │
          └──── Loop 3 (blur droit) ─────┘                                         │
                                                                                   │
          Itération k+1 : ←───────────────────────────────────────────────────────┘
```

#### Version 1 : Tâches + `taskwait` (`stencil_task.c`)

```c
#pragma omp parallel
{
  #pragma omp single
  {
    do
    {
      int end1 = 1 ;
      int end3 = 1 ;

      /* ---- Phase 1 : Blur (3 tâches indépendantes) ---- */

      /* Blur Kernel (loop 1) - côté gauche */
      #pragma omp task shared(mesh, mesh_blur, end1) firstprivate(N, threshold)
      {
        int ii ;
        for ( ii = 1 ; ii < N/10-1 ; ii++ )
        {
          double diff ;
          mesh_blur[ii] = (mesh[ii-1] + mesh[ii] + mesh[ii+1]) / 3 ;
          diff = mesh_blur[ii] - mesh[ii] ;
          if ( diff < 0 ) diff = -diff ;
          if ( diff > threshold ) end1 = 0 ;
        }
      }

      /* Blur Kernel (loop 2) - copie milieu */
      #pragma omp task shared(mesh, mesh_blur) firstprivate(N)
      {
        int ii ;
        for ( ii = N/10-1 ; ii < (int)(N*0.9-1) ; ii++ )
        {
          mesh_blur[ii] = mesh[ii] ;
        }
      }

      /* Blur Kernel (loop 3) - côté droit */
      #pragma omp task shared(mesh, mesh_blur, end3) firstprivate(N, threshold)
      {
        int ii ;
        for ( ii = (int)(N*0.9-1) ; ii < N - 1 ; ii++ )
        {
          double diff ;
          mesh_blur[ii] = (mesh[ii-1] + mesh[ii] + mesh[ii+1]) / 3 ;
          diff = mesh_blur[ii] - mesh[ii] ;
          if ( diff < 0 ) diff = -diff ;
          if ( diff > threshold ) end3 = 0 ;
        }
      }

      /* Synchronisation : attendre la fin des 3 blurs */
      #pragma omp taskwait

      /* Combiner les résultats de convergence */
      end = end1 && end3 ;

      /* ---- Phase 2 : Scale Kernel ---- */
      #pragma omp task shared(mesh, mesh_blur) firstprivate(N)
      {
        int ii ;
        for ( ii = 0 ; ii < N ; ii++ )
        {
          mesh[ii] = mesh_blur[ii] * 3.3 / 5.4 ;
        }
      }

      /* Synchronisation : attendre la fin du scale
       * avant l'itération suivante */
      #pragma omp taskwait

      niter++ ;
    }
    while( !end ) ;
  } /* fin single */
} /* fin parallel */
```

**Points clés** :
- **`end1`, `end3` séparés** : on utilise deux variables locales au lieu de `end` directement pour éviter une data race entre les tâches 1 et 3 qui écrivent toutes deux le flag de convergence.
- **1er `taskwait`** : indispensable avant d'accéder à `end1`/`end3` (écrits par les tâches) et avant de lancer le scale (qui lit `mesh_blur`).
- **2ème `taskwait`** : indispensable avant l'itération suivante (qui lit `mesh`, modifié par le scale).
- **`firstprivate(N, threshold)`** : chaque tâche reçoit une copie de ces scalaires pour éviter tout problème d'accès concurrent.

#### Version 2 : Tâches + `depend` (`stencil_taskdep.c`)

```c
/* Indices des 3 régions pour les dépendances */
int r1 = 1 ;
int r2 = N / 10 - 1 ;
int r3 = (int)(N * 0.9 - 1) ;

#pragma omp parallel
{
  #pragma omp single
  {
    do
    {
      int end1 = 1 ;
      int end3 = 1 ;

      /* Blur gauche :
       * depend(in: mesh[0])       → attend que mesh soit prêt
       * depend(out: mesh_blur[r1]) → signale écriture zone gauche */
      #pragma omp task shared(mesh, mesh_blur, end1) firstprivate(N, threshold) \
        depend(in: mesh[0]) depend(out: mesh_blur[r1])
      { /* ... blur loop 1 ... */ }

      /* Blur milieu :
       * depend(in: mesh[0])       → attend que mesh soit prêt
       * depend(out: mesh_blur[r2]) → signale écriture zone milieu */
      #pragma omp task shared(mesh, mesh_blur) firstprivate(N) \
        depend(in: mesh[0]) depend(out: mesh_blur[r2])
      { /* ... blur loop 2 ... */ }

      /* Blur droit :
       * depend(in: mesh[0])       → attend que mesh soit prêt
       * depend(out: mesh_blur[r3]) → signale écriture zone droite */
      #pragma omp task shared(mesh, mesh_blur, end3) firstprivate(N, threshold) \
        depend(in: mesh[0]) depend(out: mesh_blur[r3])
      { /* ... blur loop 3 ... */ }

      /* Scale :
       * depend(in: mesh_blur[r1], mesh_blur[r2], mesh_blur[r3])
       *   → attend les 3 tâches de blur automatiquement !
       * depend(out: mesh[0])
       *   → les blurs de l'itération SUIVANTE attendront
       *     que le scale soit terminé. */
      #pragma omp task shared(mesh, mesh_blur) firstprivate(N) \
        depend(in: mesh_blur[r1], mesh_blur[r2], mesh_blur[r3]) \
        depend(out: mesh[0])
      { /* ... scale loop ... */ }

      /* Un taskwait reste nécessaire pour lire end1/end3
       * avant le test de convergence */
      #pragma omp taskwait

      end = end1 && end3 ;
      niter++ ;
    }
    while( !end ) ;
  }
}
```

**Avantages de `depend` par rapport à `taskwait`** :

1. **Ordonnancement plus fin** : les dépendances sont exprimées entre tâches individuelles, pas entre phases entières. Le runtime peut potentiellement chevaucher le scale de l'itération k avec les blurs de l'itération k+1 (si on enlève le `taskwait` du test de convergence).

2. **Graphe de dépendances explicite** :
   - Les 3 blurs ont `depend(in: mesh[0])` → ils attendent que le scale précédent ait fini d'écrire `mesh`.
   - Le scale a `depend(in: mesh_blur[r1], mesh_blur[r2], mesh_blur[r3])` → il attend les 3 blurs.
   - Pas besoin de `taskwait` pour cet ordonnancement : les `depend` suffisent.

3. **Un `taskwait` reste nécessaire** uniquement pour lire `end1`/`end3` (le thread `single` doit attendre pour tester la condition de convergence de la boucle `do...while`).

#### Résultats de performance

**Configuration** : 1 000 000 éléments, seed 42, 4 threads

| Version | Temps (sec) | Itérations |
|---|---|---|
| Séquentielle | **0.008** | 5 |
| Tasks + `taskwait` | 0.042 | 5 |
| Tasks + `depend` | **0.022** | 5 |

**Analyse** :
- Les 3 versions produisent le **même résultat** (MESH identique), validant la correction.
- Le séquentiel est le plus rapide car le nombre d'itérations est faible (5) et le surcoût de création des threads/tâches n'est pas amorti.
- La version `depend` est **2x plus rapide** que la version `taskwait` car le runtime dispose de plus d'informations pour ordonnancer les tâches efficacement.
- Pour des maillages plus grands ou plus d'itérations, l'avantage du parallélisme serait plus marqué, car les boucles de blur et scale deviendraient le goulot d'étranglement.
