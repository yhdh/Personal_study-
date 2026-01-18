#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void* helloWorld(void * arg){
    long arg_passed = (long)arg; 
    char buf[100]; 
    sprintf(buf, "hello world, num recievde: %li, self: %p\n", arg_passed, pthread_self()); 
    printf("%s", buf);
    sleep(1);
    return NULL; 
}

int main( int argc, char **argv){
    if( argc < 2){
        perror("Usage : n_threads nombre de threads \n");
    } 
    int n = atoi(argv[1]);
    pthread_t newthread[n]; 
    long i; 
    for( i = 0; i < n; i++){
        pthread_create(&newthread[i], NULL, helloWorld, (void *)i);  
        printf("ID du thread : %lu\n", (unsigned long)newthread[i]); 
    }
    for( int i = 0; i < n; i++){
        pthread_join(newthread[i],NULL); 
    }
    return 0; 
}