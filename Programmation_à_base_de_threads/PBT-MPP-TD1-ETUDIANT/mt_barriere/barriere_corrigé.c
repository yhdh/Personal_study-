#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct
{
      pthread_mutex_t mutex;
      pthread_cond_t  cond;

      int num;        // nombre total de threads attendus
      int count;      // nombre de threads arrivés sur la barrière
      int generation; // pour rendre la barrière réutilisable
} my_barrier_t;

/* Initialise une barrière pour num threads */
int my_barrier_init(my_barrier_t *barrier, int num)
{
      if (!barrier || num <= 0) return -1;

      barrier->num = num;
      barrier->count = 0;
      barrier->generation = 0;

      if (pthread_mutex_init(&barrier->mutex, NULL) != 0) return -1;
      if (pthread_cond_init(&barrier->cond, NULL) != 0) return -1;

      return 0;
}

/* Bloque tant que tous les threads ne sont pas arrivés */
int my_barrier_wait(my_barrier_t *barrier)
{
      if (!barrier) return -1;

      pthread_mutex_lock(&barrier->mutex);

      int my_gen = barrier->generation;

      barrier->count++;

      if (barrier->count == barrier->num)
      {
            // dernier arrivé : on libère tout le monde et on réarme la barrière
            barrier->count = 0;
            barrier->generation++;
            pthread_cond_broadcast(&barrier->cond);
            pthread_mutex_unlock(&barrier->mutex);
            return 1; // optionnel: indique "thread leader"
      }

      while (my_gen == barrier->generation)
      {
            pthread_cond_wait(&barrier->cond, &barrier->mutex);
      }

      pthread_mutex_unlock(&barrier->mutex);
      return 0;
}

my_barrier_t barrier;

void * run(void * arg)
{
    long rank = (long)arg;
    int i;
    for(i = 0 ; i < 5 ; i++)
    {
        sleep( rand()%5 );
        printf("*Iter %d,Thread %ld* AVANT \n", i, rank);
        my_barrier_wait(&barrier);
        printf("*Iter %d,Thread %ld* APRES \n", i, rank);
    }
    return NULL;
}

#define NUM_THREADS 5

int main(int argc, char ** argv)
{
   pthread_t threads[NUM_THREADS];
   long i;

   my_barrier_init(&barrier, NUM_THREADS);

   srand((unsigned)time(0));

   for(i=0; i < NUM_THREADS; ++i)
      pthread_create(&threads[i], NULL, run, (void*)i);

   for(i=0; i < NUM_THREADS; ++i)
      pthread_join(threads[i], NULL);

   return 0;
}