#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int valB = 0;
int ready = 0;  // ✅ Flag pour indiquer que B a terminé

int work(int valeur)
{   
   sleep(rand()%5);
   return valeur+1;
}

void *runA(void *arg)
{
   int valA = 0;
   
   valA = work(valA);
    
   pthread_mutex_lock(&mutex);
   
   while (!ready) {  // ✅ Vérifie le flag dans une boucle
       printf("*A* En attente du résultat de B\n");
       pthread_cond_wait(&cond, &mutex);
   }
   
   pthread_mutex_unlock(&mutex);
   
   printf("*A* Valeur de A: %d Valeur de B: %d\n", valA, valB);
   return NULL;
}

void *runB(void *arg)
{
   valB = work(valB);
   
   pthread_mutex_lock(&mutex);    // ✅ Protéger le flag
   ready = 1;                     // ✅ Marquer comme terminé
   pthread_cond_signal(&cond);
   pthread_mutex_unlock(&mutex);  // ✅ Libérer le mutex
   
   printf("*B* Valeur calculée %d \n", valB);
   
   return NULL;
}

int main(int argc, char **argv)
{  
   pthread_t threadA, threadB;
   
   srand(time(0));
   
   pthread_create(&threadA, NULL, runA, NULL);
   pthread_create(&threadB, NULL, runB, NULL);
      
   pthread_join(threadA, NULL);
   pthread_join(threadB, NULL);    
   
   return 0;
}

