#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * Q2: Réécriture de prog_omp.c avec threads POSIX
 * Comportement identique au programme OpenMP original
 */

typedef struct 
{
    int thread_id;   /* Identifiant du thread (0 = maître) */
    int N;           /* Valeur de N pour la boucle */
} thread_arg_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
int sum = 0; 

void *sum_thread(void *arg)
{
    thread_arg_t *p = (thread_arg_t *)arg;
    int sum_loc = 0;

    /* Équivalent de #pragma omp master */
    if (p->thread_id == 0) {
        printf("Je suis le maitre !\n");
    }

    /* Chaque thread fait la même boucle (comme dans OpenMP sans for) */
    for (int i = 0; i < p->N; i++) {
        sum_loc += i;
    }

    /* Équivalent de #pragma omp critical */
    pthread_mutex_lock(&mutex); 
    sum += sum_loc; 
    pthread_mutex_unlock(&mutex); 

    free(p);
    return NULL; 
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <nthreads> <N>\n", argv[0]); 
        return 1;
    }

    int nthreads = atoi(argv[1]); 
    int N = atoi(argv[2]); 

    pthread_t tids[nthreads];

    for (int i = 0; i < nthreads; i++) {
        thread_arg_t *p = (thread_arg_t *)malloc(sizeof(thread_arg_t)); 
        p->thread_id = i; 
        p->N = N; 
        pthread_create(&tids[i], NULL, sum_thread, (void *)p); 
    }

    for (int i = 0; i < nthreads; i++) { 
        pthread_join(tids[i], NULL);  
    }
    
    printf("sum = %d\n", sum); 

    return 0;
}