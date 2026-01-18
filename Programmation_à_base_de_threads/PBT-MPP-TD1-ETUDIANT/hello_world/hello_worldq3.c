#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct{
    int numero; 
    char * str; 
} to_print; 

void *hello_world(void *arg)
{
   to_print *tp = (to_print *)arg; 
   printf("%s %d \n", tp->str, tp->numero);   
    return NULL; 

}

int main(int argc, char *argv[]){

    if(argc < 2){
       fprintf(stderr, "Usage: %s <nombre de threads>\n", argv[0]); 
       exit(1); 
    }
    int nthreads = atoi(argv[1]); 

    pthread_t pids[nthreads]; 

    
    

    for(int i =0; i < nthreads; i++){
        to_print *tp = (to_print *)malloc(sizeof(to_print)); 
        tp->numero = i+1; 
        tp->str = (char*)malloc(32*sizeof(char)); 
        sprintf(tp->str, "Hello world, my rank is:"); 
        if(pthread_create(&pids[i], NULL, hello_world, (void *)tp)){
            perror("pthred_create"); 
            exit(EXIT_FAILURE); 
        } 
    }

    for(int i = 0; i < nthreads; i++){
        if(pthread_join(pids[i], NULL)){
            perror("pthread_join"); 
            exit(EXIT_FAILURE); 
        }
    }

    return 0; 

}
