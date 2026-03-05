# Rapport TP3 — Parallélisation par tâches OpenMP

---

## I. Calcul de π par Monte Carlo

### Q.1 — Programme parallèle à base de tâches OpenMP 3.0

#### Principe

On tire aléatoirement N points $(x, y) \in [0,1]^2$. Un point est dans le quart de cercle si $x^2 + y^2 \leq 1$. La proportion de points dans le cercle tend vers $\frac{\pi}{4}$, d'où :

$$\pi \approx 4 \times \frac{\text{nb points dans le cercle}}{\text{nb total de points}}$$

#### Implémentation (`Pi_MT/pi_sequentiel.c`)

La parallélisation utilise des **tâches OpenMP** selon le schéma suivant :

```c
#pragma omp parallel
{
  #pragma omp single
  {
    for (int t = 0; t < nthreads * N_TASKS_PER_THREAD; t++)
    {
      #pragma omp task firstprivate(t)
      {
        unsigned int seed = (unsigned int)(t * 31 + 2020);
        uint64_t local_count = 0;
        double x, y;

        for (int k = 0; k < N_TRIALS_PER_TASK; k++)
        {
          x = rand_r(&seed) / (double)RAND_MAX;
          y = rand_r(&seed) / (double)RAND_MAX;
          local_count += (((x * x) + (y * y)) <= 1.0);
        }

        #pragma omp atomic
        count += local_count;
      }
    }
  }
}
```

#### Points clés de l'implémentation

1. **`#pragma omp single`** : un seul thread crée l'ensemble des tâches ; les autres threads les exécutent au fur et à mesure. Sans `single`, chaque thread créerait ses propres tâches, multipliant le travail.

2. **`rand_r(&seed)` au lieu de `rand()`** : la fonction `rand()` utilise un état global partagé et n'est pas thread-safe. `rand_r()` prend une graine locale en paramètre, éliminant toute data race.

3. **`firstprivate(t)`** : chaque tâche reçoit sa propre copie de `t` pour calculer une graine unique `seed = t * 31 + 2020`, garantissant des séquences aléatoires différentes.

4. **Variables `x`, `y` locales** à chaque tâche (déclarées dans le bloc `task`), évitant tout conflit entre threads.

5. **`#pragma omp atomic`** : accumulation thread-safe du compteur `count` sans utiliser de section critique (plus léger qu'un `critical`).

6. **Nombre total de tâches** = `nthreads × N_TASKS_PER_THREAD`, chacune effectuant `N_TRIALS_PER_TASK` tirages.

#### Résultats (4 threads, 1000 tâches/thread, 10 000 tirages/tâche)

| Métrique | Valeur |
|---|---|
| Points totaux | 40 000 000 |
| Points dans le cercle | 31 415 625 |
| π approximé | **3.141563** |
| Temps d'exécution | 0.34 s |

L'approximation est très proche de la valeur réelle π ≈ 3.141593.

---

## II. Sparse Matrix Vector Kernel (SpMV)

### Format CSR (Compressed Sparse Row)

Une matrice creuse de taille N×N est stockée par trois tableaux :
- **`values`** (taille NNZ) : valeurs non nulles, rangées ligne par ligne.
- **`JA`** (taille NNZ) : indice de colonne de chaque valeur.
- **`IA`** (taille N+1) : `IA[i]` pointe vers le début de la ligne `i` dans `values` et `JA`.

Pour la ligne `i`, les éléments non nuls sont aux indices `IA[i]` à `IA[i+1]-1` dans `values` et `JA`.

---

### Q.2 — Produit matrice creuse–vecteur séquentiel

#### Implémentation (`CSRMatrix.c`)

```c
void mult_CSR(CSRMatrix_t* A, double const* x, double* y)
{
  int N = A->m_nrows;
  uint64_t* ia = A->m_ia;
  uint64_t* ja = A->m_ja;
  double*   val = A->m_values;

  for (int i = 0; i < N; i++) {
    double sum = 0.0;
    for (uint64_t k = ia[i]; k < ia[i+1]; k++) {
      sum += val[k] * x[ja[k]];
    }
    y[i] = sum;
  }
}
```

#### Explication

Pour chaque ligne `i` :
1. On parcourt les éléments non nuls de cette ligne, indexés de `ia[i]` à `ia[i+1]-1`.
2. Pour chaque élément `k`, on multiplie la valeur `val[k]` par la composante correspondante du vecteur `x[ja[k]]`.
3. La somme est accumulée dans `y[i]`.

Le format CSR permet un accès séquentiel aux valeurs et aux indices de colonnes, ce qui est favorable au cache pour le parcours ligne par ligne.

#### Résultats séquentiels (grille 1000×1000, 20 itérations)

| Métrique | Valeur |
|---|---|
| NROWS | 1 000 000 |
| NNZ | 4 996 000 |
| NORME Y | 3.70 |
| Temps moyen / itération | 6.9 ms |
| Performance | 1 441 MFlops |

---

### Q.3 — Parallélisation par tâches OpenMP 4.0

#### Implémentation (`CSRMatrix.c`)

```c
void mult_CSR_task(CSRMatrix_t* A, double const* x, double* y, int nb_tasks)
{
  int N = A->m_nrows;
  uint64_t* ia = A->m_ia;
  uint64_t* ja = A->m_ja;
  double*   val = A->m_values;
  int block = (N + nb_tasks - 1) / nb_tasks;

  #pragma omp parallel
  {
    #pragma omp single
    {
      for (int t = 0; t < nb_tasks; t++) {
        #pragma omp task firstprivate(t)
        {
          int istart = t * block;
          int iend   = istart + block;
          if (iend > N) iend = N;

          for (int i = istart; i < iend; i++) {
            double sum = 0.0;
            for (uint64_t k = ia[i]; k < ia[i+1]; k++) {
              sum += val[k] * x[ja[k]];
            }
            y[i] = sum;
          }
        }
      }
    }
  }
}
```

#### Explication

1. **Découpage en blocs** : les N lignes sont réparties en `nb_tasks` blocs de taille `block = ⌈N / nb_tasks⌉`. Chaque tâche traite les lignes `[t×block, (t+1)×block[`.

2. **`#pragma omp parallel` + `#pragma omp single`** : un seul thread crée les tâches ; tous les threads (y compris le créateur) participent à leur exécution.

3. **`#pragma omp task firstprivate(t)`** : chaque tâche reçoit sa propre copie de `t` pour calculer ses bornes `istart`/`iend`.

4. **Pas de synchronisation sur `y`** : chaque tâche écrit dans un sous-ensemble disjoint de `y` (lignes différentes), il n'y a donc aucun conflit d'écriture.

5. **Surcharge de tâches** : on utilise `nb_tasks = 4 × nthreads` plutôt que `nthreads` pour améliorer l'équilibrage. Si certaines tâches terminent plus vite (lignes avec moins de non-zéros), les threads libérés prennent immédiatement les tâches restantes.

#### Résultats (grille 1000×1000, 20 itérations, 4 threads)

| Métrique | Séquentiel | Tâches OpenMP |
|---|---|---|
| NORME Y | 3.70 | 3.70 |
| Temps moyen | 6.9 ms | 6.1 ms |
| MFlops | 1 441 | 1 634 |
| **Speedup** | 1.0x | **1.13x** |

Les deux versions produisent le même résultat (NORME Y identique), ce qui valide la correction de l'implémentation parallèle.

---

### Q.4 — Avantages de la parallélisation par tâches

La parallélisation par tâches OpenMP du SpMV apporte plusieurs avantages par rapport à une parallélisation classique par `#pragma omp parallel for` :

#### 1. Équilibrage dynamique de charge

Dans un Laplacien 2D, les lignes de bord ont 3–4 éléments non nuls tandis que les lignes intérieures en ont 5. Un découpage statique (par `schedule(static)`) peut créer un déséquilibre si les premiers ou derniers threads héritent de lignes plus légères ou plus lourdes. Avec les tâches, le runtime OpenMP distribue dynamiquement les tâches aux threads libres : dès qu'un thread termine une tâche, il en prend une nouvelle dans la file d'attente. En créant plus de tâches que de threads (surcharge), l'équilibrage est naturellement meilleur.

#### 2. Flexibilité et composabilité

Les tâches permettent de composer facilement le parallélisme avec d'autres parties du programme. On peut imbriquer des tâches ou les combiner avec d'autres patterns (producteur-consommateur, pipelines) sans restructurer le code. C'est particulièrement utile dans un solveur itératif où le SpMV n'est qu'une étape parmi d'autres.

#### 3. Granularité ajustable

Le paramètre `nb_tasks` permet de contrôler finement la granularité :
- **Peu de tâches** (= nthreads) : faible overhead mais équilibrage limité.
- **Beaucoup de tâches** (>> nthreads) : meilleur équilibrage mais overhead de création/ordonnancement plus élevé.

On peut ainsi trouver le compromis optimal pour chaque configuration matrice/architecture.

#### 4. Compatibilité avec des matrices irrégulières

Pour des matrices issues de maillages non structurés (nombre de non-zéros très variable par ligne), l'approche par tâches est particulièrement adaptée car elle absorbe naturellement l'irrégularité de la charge.

#### Limites observées

Le speedup modeste (1.13x avec 4 threads) s'explique par le fait que le SpMV est un algorithme **memory-bound** : le facteur limitant n'est pas la puissance de calcul mais la **bande passante mémoire**. Chaque élément non nul nécessite un accès indirect à `x[ja[k]]`, ce qui provoque des défauts de cache. Ajouter des threads ne résout pas ce goulot d'étranglement mémoire. Le speedup réel dépend donc de la bande passante disponible plutôt que du nombre de cœurs.

---

## Compilation et exécution

### Pi Monte Carlo
```bash
cd Pi_MT
make clean && make
OMP_NUM_THREADS=4 ./pi.exe
```

### SpMV
```bash
cd SpMV
make clean && make
OMP_NUM_THREADS=4 ./spmv.exe 1000 1000 20
```

Le premier et deuxième arguments sont les dimensions de la grille (nx, ny), le troisième est le nombre d'itérations.
