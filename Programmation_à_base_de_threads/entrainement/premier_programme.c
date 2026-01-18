#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


void* myTurn(){
    for(int i = 0; i < 8; i++){
        sleep(1); 
        printf("My turn \n"); 
    }
    return NULL; 
}
void yourTurn(){
    for(int i = 0; i < 5; i++){
        sleep(1); 
        printf("Your turn \n"); 
    }
}

int main(){
    pthread_t newThread; 
    pthread_create(&newThread, NULL, myTurn, NULL); 
    //myTurn(); 
    yourTurn(); 
    pthread_join(newThread, NULL); 

}