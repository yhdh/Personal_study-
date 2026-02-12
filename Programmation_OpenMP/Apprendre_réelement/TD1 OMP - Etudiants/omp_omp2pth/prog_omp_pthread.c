#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int N; 
    int threadNumber; 
}calcul; 

void * calculThread(void * arg){
    calcul * cal = malloc(sizeof(calcul)); 
    cal = (calcul *)arg; 
    int N = cal->N; 
    int thread = cal->threadNumber;
    if(cal->threadNumber == 0){
        printf("Je suis le maitre !\n"); 
    }
    int sum_loc = 0;  
    for(int i = 0 ; i < N ; i++)
	{
	    sum_loc += i;
	}
    free(cal); 
    return (void *)sum_loc; 
}

int main(int argc, char** argv){
    if (argc < 3){
        fprintf(stderr,"Usage : %s <nombre de thread> <nombre à calculé>\n",argv[0]);
        exit(1); 
    }

    int nthread = atoi(argv[1]); 
    int n = atoi(argv[2]); 

    pthread_t pids[nthread]; 
    
    
    for(int i =0; i < nthread; i++){
        calcul * cal = malloc(sizeof(calcul));
        cal->N = n; 
        cal->threadNumber = i; 
        pthread_create(&pids[i], NULL, calculThread, (void*)cal); 
        
    }
    int sum = 0; 
    for(int i = 0; i < nthread; i++){
        void *result; 
        pthread_join(pids[i], &result); 
        sum += (int)result; 
    }
    printf("sum = %d\n", sum); 
    return 0; 
}
