/*
 * Q3: Programme OpenMP avec boucle for parallélisée (distribuée sur les threads)
 * Q4: La directive #pragma omp critical assure que sum n'est pas modifiée simultanément
 * 
 * Ici la boucle for est distribuée : chaque thread traite une partie des itérations
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int sum = 0;

#pragma omp parallel shared(sum, N)
    {
        int sum_loc = 0;

#pragma omp master
        {
            printf("Je suis le maitre ! (thread %d sur %d)\n", 
                   omp_get_thread_num(), omp_get_num_threads());
        }

        /* Q3: #pragma omp for distribue les itérations entre les threads */
#pragma omp for
        for (int i = 0; i < N; i++)
        {
            sum_loc += i;
        }

        /* Q4: #pragma omp critical assure l'exclusion mutuelle */
#pragma omp critical
        {
            sum += sum_loc;
        }
    }

    printf("sum = %d\n", sum);
    printf("Valeur attendue: %d\n", (N - 1) * N / 2);

    return 0;
}
