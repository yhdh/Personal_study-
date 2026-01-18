#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define MAX_VAL 1000

int maxv_mt = 0;

/* Remplace le mutex par un sémaphore (binaire) */
sem_t sem_max;

typedef struct
{
    int nelt;
    int *tab;
} tableau;

int max_seq(int *tab, int nelt)
{
    int i, maxv;

    maxv = 0;
    for (i = 0; i < nelt; i++)
    {
        if (tab[i] > maxv)
        {
            maxv = tab[i];
        }
    }

    return maxv;
}

void *thread_calcul_max(void *arg)
{
    tableau *tb = (tableau *)arg;
    int result = max_seq(tb->tab, tb->nelt);

    printf("%d\n", result);

    sem_wait(&sem_max);
    if (result > maxv_mt)
        maxv_mt = result;
    sem_post(&sem_max);

    free(tb->tab);
    free(tb);
    return NULL;
}

int main(int argc, char **argv)
{
    int nthreads, nelt, i, t, maxv_seq, t_fin, pas;
    int *tab;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <nelt> <nthreads>\n", argv[0]);
        return 1;
    }

    nelt = atoi(argv[1]);     /* Taille du tableau */
    nthreads = atoi(argv[2]); /* Nombre de threads a creer */

    if (nelt <= 0 || nthreads <= 0)
    {
        fprintf(stderr, "Erreur: nelt et nthreads doivent être > 0\n");
        return 1;
    }
    if (nthreads > nelt)
        nthreads = nelt;

    pas = nelt / nthreads;

    /* Creation du tableau et remplissage aleatoire */
    tab = (int *)malloc(nelt * sizeof(int));
    if (!tab)
    {
        perror("malloc");
        return 1;
    }

    srand(nelt);
    for (i = 0; i < nelt; i++)
    {
        tab[i] = 1 + (rand() % MAX_VAL);
    }

    /* Init sémaphore binaire (équivalent mutex) */
    if (sem_init(&sem_max, 0, 1) != 0)
    {
        perror("sem_init");
        free(tab);
        return 1;
    }

    t = 0;
    t_fin = 0;

    pthread_t pids[nthreads];
    for (i = 0; i < nthreads; i++)
    {
        t = i * pas;

        /* dernier thread prend le reste */
        if (i == nthreads - 1)
            t_fin = nelt;
        else
            t_fin = t + pas;

        tableau *tb = (tableau *)malloc(sizeof(tableau));
        if (!tb)
        {
            perror("malloc");
            // join ceux déjà lancés
            for (int k = 0; k < i; k++)
                pthread_join(pids[k], NULL);
            sem_destroy(&sem_max);
            free(tab);
            return 1;
        }

        tb->nelt = t_fin - t;
        tb->tab = (int *)malloc(sizeof(int) * tb->nelt);
        if (!tb->tab)
        {
            perror("malloc");
            free(tb);
            for (int k = 0; k < i; k++)
                pthread_join(pids[k], NULL);
            sem_destroy(&sem_max);
            free(tab);
            return 1;
        }

        for (int j = t; j < t_fin; j++)
        {
            tb->tab[j - t] = tab[j];
        }

        pthread_create(&pids[i], NULL, thread_calcul_max, (void *)tb);
    }

    for (i = 0; i < nthreads; i++)
    {
        pthread_join(pids[i], NULL);
    }

    sem_destroy(&sem_max);

    /* Recherche du max en sequentiel pour verification => maxv_seq */
    maxv_seq = max_seq(tab, nelt);

    if (maxv_seq == maxv_mt)
    {
        printf("PASSED\n");
    }
    else
    {
        printf("FAILED\n");
        printf("Valeur correcte : %d\n", maxv_seq);
        printf("Votre valeur    : %d\n", maxv_mt);
    }

    free(tab);

    return 0;
}