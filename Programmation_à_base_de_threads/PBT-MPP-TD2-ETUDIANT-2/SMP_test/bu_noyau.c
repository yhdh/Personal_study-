/*
 * bu_noyau.c — Test SMP avec bibliothèque NOYAU (Pthread)
 *
 * Chaque thread fait un calcul CPU-intensif.
 * Avec Pthread, chaque thread = un thread noyau → vrai parallélisme.
 *
 * Compiler :
 *   gcc -O2 -pthread bu_noyau.c -o bu_noyau
 *
 * Exécuter :
 *   time ./bu_noyau
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#define ITERATIONS 500000000L

typedef struct {
    int id;
    double result;
} thread_arg_t;

void *travail_cpu(void *arg)
{
    thread_arg_t *ta = (thread_arg_t *)arg;
    double result = 0.0;

    for (long i = 0; i < ITERATIONS; i++)
        result += i * 0.000001;

    ta->result = result;
    return NULL;
}

double get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

int main(int argc, char **argv)
{
    int nthreads = (argc > 1) ? atoi(argv[1]) : sysconf(_SC_NPROCESSORS_ONLN);

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║   TEST SMP — BIBLIOTHÈQUE NOYAU (Pthread)   ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║  Threads       : %-4d                        ║\n", nthreads);
    printf("║  Cœurs dispo   : %-4ld                        ║\n", sysconf(_SC_NPROCESSORS_ONLN));
    printf("║  Itérations    : %ld / thread          ║\n", ITERATIONS);
    printf("╚══════════════════════════════════════════════╝\n\n");

    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    thread_arg_t *args = malloc(nthreads * sizeof(thread_arg_t));

    double start = get_time();

    /* Créer les threads */
    for (int i = 0; i < nthreads; i++) {
        args[i].id = i;
        pthread_create(&threads[i], NULL, travail_cpu, &args[i]);
    }

    /* Attendre la fin */
    for (int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

    double elapsed = get_time() - start;

    printf("═══ RÉSULTATS ═══\n");
    printf("  Temps réel     : %.2f s\n", elapsed);
    printf("  Temps attendu  : %.2f s (si parallélisme réel)\n", elapsed);
    printf("\n");
    printf("  → Utilisez 'time ./bu_noyau' pour voir :\n");
    printf("     real ≈ %.1fs   user ≈ %.1fs (user ≈ %d × real = SMP ✅)\n",
           elapsed, elapsed * nthreads, nthreads);

    free(threads);
    free(args);
    return 0;
}
