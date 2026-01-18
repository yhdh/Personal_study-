#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void *hello_world(void *arg)
{
    int n = (int)arg; 
    printf("Hello world : %d \n", n ); 
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
        pthread_create(&pids[i], NULL, hello_world, (void *)i); 
    }

    for(int i = 0; i < nthreads; i++){
        pthread_join(pids[i], NULL); 
    }

    return 0; 

}
