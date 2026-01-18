// En créant la chaîne (« Hello... » + identifiant) avant de la passer au
// thread à créer (utilisez la fonction sprintf)

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int NB_THREADS;

typedef struct
{
    int id; 
    char *str;
} to_print;

void *print_hello_world(void *arg)
{
    to_print *tp = (to_print *)arg;
    printf("%s %i \n", tp->str, tp->id);
    return NULL;
}

int main(int argc, char **argv)
{
    NB_THREADS = atoi(argv[1]);
    pthread_t *pids = (pthread_t *)malloc(NB_THREADS * sizeof(pthread_t));

    // Creation des threads
    for (int i = 0; i < NB_THREADS; i++)
    {
        to_print *tp = (to_print *)malloc(sizeof(to_print));
         
        tp->str = (char *)malloc(32 * sizeof(char));
        
        tp->id = i+1; 
        sprintf(tp->str, "Hello, my rank is : ");

        if (pthread_create(&(pids[i]), NULL, print_hello_world, (void *)tp))
        {
            perror("pthread create");
            exit(EXIT_FAILURE);
        }
    }

    // Attente des threads
    for (int i = 0; i < NB_THREADS; i++)
    {
        pthread_join(pids[i], NULL);
    }

    free(pids);
    return 0;
}
