/*
 * Q5: OpenMP propose la clause reduction pour calculer sum de manière
 *     plus simple et plus efficace que critical
 * 
 * La clause reduction(+:sum) :
 *   - Crée une copie locale de sum pour chaque thread (initialisée à 0 pour +)
 *   - Chaque thread accumule dans sa copie locale
 *   - À la fin de la région parallèle, toutes les copies sont combinées automatiquement
 * 
 * Avantages par rapport à critical :
 *   - Plus simple à écrire
 *   - Plus efficace (pas de verrou à chaque accès)
 *   - Moins de risque d'erreur
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

#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++)
    {
        sum += i;
    }

    printf("sum = %d\n", sum);
    printf("Valeur attendue: %d\n", (N - 1) * N / 2);

    return 0;
}
