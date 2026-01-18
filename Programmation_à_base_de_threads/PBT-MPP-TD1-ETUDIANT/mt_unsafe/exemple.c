#include <pthread.h>
#include <stdio.h>


#define COUNT_PER_THREAD 10000
#define NUM_THREADS 10
pthread_mutex_t mutex; 

unsigned long volatile count = 0;

void *  parallel_count(void *args){
   int i;
   
   for(i=0; i < COUNT_PER_THREAD; ++i){
      pthread_mutex_lock(&mutex); 
      ++count;
      pthread_mutex_unlock(&mutex); 
   }
   
   return NULL;
}



int main(int argc, char** argv)
{
   int i;
   pthread_t pid[NUM_THREADS];

   pthread_mutex_init(&mutex, NULL); 
   
   
   for(i = 0; i < NUM_THREADS; ++i){
      pthread_create(&pid[i], NULL, parallel_count, NULL);
   }
   
   for(i = 0; i < NUM_THREADS; ++i){
      pthread_join(pid[i], NULL);
   }
   
   printf("Valeur finale de count %ld \n", count);
   
   pthread_mutex_destroy(&mutex); 

   return 0;
}
