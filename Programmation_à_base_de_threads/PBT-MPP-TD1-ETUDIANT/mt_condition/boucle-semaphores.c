#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

sem_t sa; 
sem_t sb; 

int valB = 0;


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
      printf("*A* En attente du résultat de B\n");
      sem_wait(&sa);
      printf("*A* Valeur de A: %d Valeur de B: %d\n", valA, valB);
      sem_post(&sb); 
   }
   return NULL;
}

void * runB(void * arg)
{
   (void)arg;

   while (1)
   {
      sem_wait(&sb);
      valB = work(valB); 
      printf("*B* Valeur calculée %d \n", valB);
      sem_post(&sa); 
   }
   return NULL;
}

int main(void)
{
    sem_init(&sa,0,0); 
    sem_init(&sb,0,1);
   pthread_t threadA, threadB;
   
   srand(time(0));
   
   pthread_create(&threadA, NULL, runA, NULL);
   pthread_create(&threadB, NULL, runB, NULL);
      
   pthread_join(threadA, NULL);
   pthread_join(threadB, NULL);    
   
   sem_destroy(&sa); 
   sem_destroy(&sb); 
   return 0;
}