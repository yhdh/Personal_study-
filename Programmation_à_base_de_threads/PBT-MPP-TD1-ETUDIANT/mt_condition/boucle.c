#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int valB = 0;
int ready = 0;

int work(int valeur)
{
   /* Fait semblant de bosser */
   sleep((unsigned)arc4random_uniform(5));
   return valeur + 1;
}

void * runA(void * arg)
{
   (void)arg;
   int valA = 0;

   while (1)
   {
      valA = work(valA);

      pthread_mutex_lock(&mutex);

      while (ready == 0) {
         printf("*A* En attente du résultat de B\n");
         pthread_cond_wait(&cond, &mutex);
      }

      int localB = valB;  // consommer sous protection
      ready = 0;          // buffer vidé
      pthread_cond_signal(&cond); // réveiller B s'il attend "buffer vide"

      pthread_mutex_unlock(&mutex);

      /* Les deux valeurs devraient toujours être égales */
      printf("*A* Valeur de A: %d Valeur de B: %d\n", valA, localB);
   }
   return NULL;
}

void * runB(void * arg)
{
   (void)arg;

   while (1)
   {
      // Attendre que A ait consommé la précédente valeur
      pthread_mutex_lock(&mutex);
      while (ready == 1) {
         pthread_cond_wait(&cond, &mutex);
      }
      int base = valB; // snapshot protégé
      pthread_mutex_unlock(&mutex);

      int newB = work(base);

      pthread_mutex_lock(&mutex);
      valB = newB;
      ready = 1;                 // buffer plein
      pthread_cond_signal(&cond); // réveiller A
      int localB = valB;
      pthread_mutex_unlock(&mutex);

      printf("*B* Valeur calculée %d \n", localB);
   }
   return NULL;
}

int main(void)
{
   pthread_t threadA, threadB;

   pthread_create(&threadA, NULL, runA, NULL);
   pthread_create(&threadB, NULL, runB, NULL);

   pthread_join(threadA, NULL);
   pthread_join(threadB, NULL);

   return 0;
}