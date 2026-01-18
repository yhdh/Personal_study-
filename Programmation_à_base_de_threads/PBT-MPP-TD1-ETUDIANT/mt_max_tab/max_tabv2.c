#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_VAL 1000

int maxv_mt = 0; 

pthread_mutex_t mutex; 

typedef struct 
{
    int nelt; 
    int *tab; 
}tableau;


int max_seq(int *tab, int nelt)
{
    int i, maxv;

    maxv = 0;
    for( i = 0 ; i < nelt ; i++ )
    {
	if (tab[i] > maxv)
	{
	    maxv = tab[i];
	}
    }

    return maxv;
}
void *thread_calcul_max(void *arg){
    tableau *tb = (tableau *)arg; 
    int result = max_seq(tb->tab, tb->nelt); 
    printf("%d\n",result); 
    pthread_mutex_lock(&mutex);
    if (result > maxv_mt) maxv_mt = result; 
    pthread_mutex_unlock(&mutex); 
    free(tb); 
    return NULL; 

}

int main(int argc, char **argv)
{
    int nthreads, nelt, i, t, maxv_seq,  t_fin, pas ;
    int *tab;

    nelt     = atoi(argv[1]); /* Taille du tableau */
    nthreads = atoi(argv[2]); /* Nombre de threads a creer */
    pas = nelt/nthreads; 

    /* Creation du tableau et remplissage aleatoire */
    tab = (int*)malloc(nelt*sizeof(int));

    srand(nelt);
    for( i = 0 ; i < nelt ; i++)
    {
	tab[i] = 1 + (rand() % MAX_VAL);
    }

    
    t = 0; 
    t_fin = 0; 
    /* A ECRIRE */
    pthread_t pids[nthreads]; 
    for(i = 0; i < nthreads; i++){
        t = i*pas; 
        if (t_fin < nelt) t_fin = t + pas; 
        else t_fin = nelt; 

        tableau *tb = (tableau *)malloc(sizeof(tableau)); 
        tb->nelt = t_fin - t; 
        tb->tab = (int *)malloc(sizeof(int)*tb->nelt); 
          
        for (int j =t; j <= t_fin; j++ ){
            tb->tab[j-t] = tab[j];  
        }
        
        pthread_create(&pids[i],NULL,thread_calcul_max, (void *)tb); 
            
    }
   for(i = 0; i < nthreads; i++){ 
        pthread_join(pids[i],NULL);  
   }

    

    /* Recherche du max en sequentiel pour verification => maxv_seq */
    maxv_seq = max_seq(tab, nelt);

    if (maxv_seq == maxv_mt)
    {
	printf("PASSED\n");
    }
    else
    {
	printf("FAILED\n");
	printf("Valeur correcte : %d\n", maxv_seq);
	printf("Votre valeur    : %d\n", maxv_mt);
    }

    free(tab);

    return 0;
}

