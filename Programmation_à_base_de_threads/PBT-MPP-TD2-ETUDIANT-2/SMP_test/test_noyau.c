#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

int ITERATION = 10000000; 

typedef struct{
    int thread_number; 
    int debut_iter; 
    int fin_iter; 
}thread_iteration; 

void* iter_thread(void *arg){
    thread_iteration *p = malloc(sizeof(thread_iteration)); 
    p = (thread_iteration *)arg; 
    for(int i = p->debut_iter; i <= p->fin_iter; i++){
        i++; 
    }
    
    return NULL; 
}


int main(int argc, char **argv){
    clock_t start, end; 
    double cpu_time_used; 

    start = clock(); 
    if(argc < 2){
        fprintf(stderr,"Usage : %s <nombre de thread>\n",argv[0]); 
    }

    int nthread = atoi(argv[1]); 

   int pas = ITERATION / nthread; 
    pthread_t pids[nthread];
    for(int i = 0; i <= nthread-1; i++){
        
        thread_iteration *p = malloc(sizeof(thread_iteration)); 
        p->thread_number = i; 
        p->debut_iter = pas*i; 
        p->fin_iter = (i == nthread-1) ? ITERATION : pas*(i+1)-1; 
        pthread_create(&pids[i],NULL,iter_thread,(void*)p); 
    }

    for(int i = 0; i <= nthread-1; i++){
        pthread_join(pids[i],NULL); 
    }

    end = clock(); 
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; 

    printf("le temps de calcul est %f \n",cpu_time_used ); 
}