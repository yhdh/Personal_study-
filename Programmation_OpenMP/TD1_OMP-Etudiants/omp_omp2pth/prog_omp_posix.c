#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


typedef struct 
{
    int i; 
    int N; 
}pthread_calcul;

pthread_mutex_t mutex; 
int sum; 

void *sum_thread(void *  arg){
    pthread_calcul *p = (pthread_calcul *)arg;

    if (p->i == 0){
        printf("Je suis le maitre !\n");
    }
    int sum_loc = 0; 
    for(int i = 0; i < p->N; i++){
        sum_loc += i; 
    }
    pthread_mutex_lock(&mutex); 
    sum += sum_loc; 
    pthread_mutex_unlock(&mutex); 
    

    return NULL; 


}

int main(int argc, char **argv){

    if(argc < 3){
        fprintf(stderr, "Usage: %s <nombre de threads> <nombre de boucle\n", argv[0]); 
    }

     

    sum = 0; 

    int N = atoi(argv[2]); 
    int nthread = atoi(argv[1]); 

    pthread_t pids[nthread];

    for (int i = 0; i < nthread; i++){
        pthread_calcul *p = (pthread_calcul *)malloc(sizeof(pthread_calcul)); 
        p->i = i; 
        p->N = N; 
        pthread_create(&pids[i], NULL,sum_thread,(void *)p); 
    }

    for (int i = 0; i < nthread; i++){ 
        pthread_join(pids[i],NULL );  
    }
    
    printf("%d \n", sum); 

}