#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct
{
    int *tab;
    int min;
    int max;
} tab_min;

#define MAX_VAL 1000
int max = 0; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

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

void *max_tab_seq(void *arg)
{
    tab_min *tab_t = (tab_min *)arg;

    // Allocation de mémoire pour stocker le résultat
    int result = max_seq(tab_t->tab, (tab_t->max - tab_t->min)); 
    pthread_mutex_lock(&mutex); 
    if (max < result)
    {
        
        max = result; 
    }
    pthread_mutex_unlock(&mutex); 
    return NULL;
}

int main(int argc, char **argv)
{
    int nthreads, nelt, i, maxv_seq ;


    nelt = atoi(argv[1]);     /* Taille du tableau */
    nthreads = atoi(argv[2]); /* Nombre de threads a creer */

    int *tab;
    /* Creation du tableau et remplissage aleatoire */
    tab = (int *)malloc(nelt * sizeof(int));

    srand(nelt);
    for (i = 0; i < nelt; i++)
    {
        tab[i] = 1 + (rand() % MAX_VAL);
    }

    /* Recherche du max de tab en contexte multithread => maxv_mt*/
    //maxv_mt = 0;

    ///////////////////////////////////////////////////////////////////////////
    /* CODE MODIFIE */
    ///////////////////////////////////////////////////////////////////////////

    pthread_t pids[nthreads];

    // on calcule la taille des portions à donner
    int range = nelt / nthreads;

    // on crée un tableau de struct a donner
    tab_min *tableau_thread = (tab_min *)malloc(nthreads * sizeof(tab_min));

    for (int i = 0; i < nthreads; i++)
    {
        tableau_thread[i].tab = tab + (i * range);
        tableau_thread[i].min = i * range;
        // Pour le dernier thread, la fin est nelt
        tableau_thread[i].max = (i == nthreads - 1) ? nelt : (i + 1) * range;
    }
    // création des threads
    for (int i = 0; i < nthreads; i++)
    {
        pthread_create(&pids[i], NULL, max_tab_seq, (void *)&tableau_thread[i]);
    }

    int *final_tab[nthreads];

    // join des threads
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(pids[i], (void **)&final_tab[i]);
    }

    /*for (int i = 0; i < nthreads; i++)
    {
        if (*final_tab[i] > maxv_mt)
        {
            maxv_mt = *final_tab[i];
        }
        free(final_tab[i]);
    }*/

    ///////////////////////////////////////////////////////////////////////////
    /* FIN CODE MODIFIE */
    ///////////////////////////////////////////////////////////////////////////

    /* Recherche du max en sequentiel pour verification => maxv_seq */
    maxv_seq = max_seq(tab, nelt);
    printf("%d \n", max);
    if (maxv_seq == max)
    {
        printf("PASSED\n");
    }
    else
    {
        printf("FAILED\n");
        printf("Valeur correcte : %d\n", maxv_seq);
        printf("Votre valeur    : %d\n", max);
    }

    free(tab);
    free(tableau_thread);

    return 0;
}
