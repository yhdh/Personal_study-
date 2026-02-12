/*
 * Q6: Recherche parallèle du max d'un tableau en OpenMP
 *     Reprise du TD1 Pthread (Q7 et Q8) en version OpenMP
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_VAL 1000

/* Version séquentielle pour vérification */
int max_seq(int *tab, int nelt)
{
    int maxv = 0;
    for (int i = 0; i < nelt; i++)
    {
        if (tab[i] > maxv)
        {
            maxv = tab[i];
        }
    }
    return maxv;
}

/* 
 * Méthode 1: Utilisation de reduction(max:)
 * C'est la méthode la plus simple et efficace avec OpenMP
 */
int max_par_reduction(int *tab, int nelt)
{
    int maxv = 0;

#pragma omp parallel for reduction(max:maxv)
    for (int i = 0; i < nelt; i++)
    {
        if (tab[i] > maxv)
        {
            maxv = tab[i];
        }
    }

    return maxv;
}

/*
 * Méthode 2: Max local + critical (équivalent Pthread avec mutex)
 */
int max_par_critical(int *tab, int nelt)
{
    int maxv_global = 0;

#pragma omp parallel shared(maxv_global)
    {
        int maxv_local = 0;

#pragma omp for
        for (int i = 0; i < nelt; i++)
        {
            if (tab[i] > maxv_local)
            {
                maxv_local = tab[i];
            }
        }

#pragma omp critical
        {
            if (maxv_local > maxv_global)
            {
                maxv_global = maxv_local;
            }
        }
    }

    return maxv_global;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <nelt> [nthreads]\n", argv[0]);
        return 1;
    }

    int nelt = atoi(argv[1]);
    if (argc >= 3) {
        omp_set_num_threads(atoi(argv[2]));
    }

    /* Création et remplissage du tableau */
    int *tab = (int *)malloc(nelt * sizeof(int));
    if (!tab) {
        perror("malloc");
        return 1;
    }

    srand(nelt);
    for (int i = 0; i < nelt; i++)
    {
        tab[i] = 1 + (rand() % MAX_VAL);
    }

    /* Tests des différentes méthodes */
    int maxv_seq = max_seq(tab, nelt);
    int maxv_reduction = max_par_reduction(tab, nelt);
    int maxv_critical = max_par_critical(tab, nelt);

    printf("=== Résultats ===\n");
    printf("Max séquentiel:           %d\n", maxv_seq);
    printf("Max parallel (reduction): %d\n", maxv_reduction);
    printf("Max parallel (critical):  %d\n", maxv_critical);

    if (maxv_seq == maxv_reduction && maxv_seq == maxv_critical)
    {
        printf("\nPASSED: Toutes les méthodes donnent le même résultat\n");
    }
    else
    {
        printf("\nFAILED: Les résultats diffèrent!\n");
    }

    free(tab);
    return 0;
}
