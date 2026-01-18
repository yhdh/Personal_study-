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
   sleep(rand()%5);
   
   return valeur+1;
}

void * runA(void * arg)
{
   int valA = 0;
   
   valA = work(valA);
    
   
   printf("*A* En attente du résultat de B\n");
   
   
   sem_wait(&sa);
   
   printf("*A* Valeur de A: %d Valeur de B: %d\n", valA, valB);
   sem_post(&sb);
   return NULL;
}


void * runB(void * arg)
{
    valB = work(valB);
   
    /*Signale à A que j'ai calculé valB*/
    printf("*B* Valeur calculée %d \n", valB);
    sem_post(&sa); 
    sem_wait(&sb);
   
   return NULL;
}


int main(int argc, char ** argv)
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
