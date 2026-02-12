/*
 * bu_utilisateur.c — Test SMP avec bibliothèque UTILISATEUR (GnuPth)
 *
 * Chaque thread fait le MÊME calcul CPU-intensif que bu_noyau.c.
 * Avec GnuPth, tous les threads partagent UN SEUL thread noyau
 * → pas de parallélisme réel, un seul cœur utilisé.
 *
 * GnuPth est coopératif : un thread garde le CPU jusqu'à ce qu'il
 * appelle pth_yield(). On insère des yield périodiques pour que
 * tous les threads progressent.
 *
 * Compiler :
 *   gcc -O2 -I../pth-2.0.7 -L../pth-2.0.7/.libs bu_utilisateur.c \
 *       -lpth -ldl -o bu_utilisateur
 *
 * Exécuter :
 *   export DYLD_LIBRARY_PATH=../pth-2.0.7/.libs:$DYLD_LIBRARY_PATH
 *   time ./bu_utilisateur
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "pth.h"

#define ITERATIONS 500000000L
#define YIELD_EVERY 10000000L  /* yield périodique pour la coopération */

typedef struct {
    int id;
    double result;
} thread_arg_t;

void *travail_cpu(void *arg)
{
    thread_arg_t *ta = (thread_arg_t *)arg;
    double result = 0.0;

    for (long i = 0; i < ITERATIONS; i++) {
        result += i * 0.000001;
        /* Yield périodique : nécessaire car GnuPth est coopératif.
           Sans yield, un seul thread monopolise le CPU. */
        if (i % YIELD_EVERY == 0)
            pth_yield(NULL);
    }

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

    /* Initialiser GnuPth */
    if (!pth_init()) {
        fprintf(stderr, "Erreur: pth_init() a échoué\n");
        return 1;
    }

    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║  TEST SMP — BIBLIOTHÈQUE UTILISATEUR (GnuPth)   ║\n");
    printf("╠══════════════════════════════════════════════════╣\n");
    printf("║  Threads       : %-4d                            ║\n", nthreads);
    printf("║  Cœurs dispo   : %-4ld                            ║\n", sysconf(_SC_NPROCESSORS_ONLN));
    printf("║  Itérations    : %ld / thread              ║\n", ITERATIONS);
    printf("╚══════════════════════════════════════════════════╝\n\n");

    pth_t *threads = malloc(nthreads * sizeof(pth_t));
    thread_arg_t *args = malloc(nthreads * sizeof(thread_arg_t));

    pth_attr_t attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_JOINABLE, TRUE);

    double start = get_time();

    /* Créer les threads */
    for (int i = 0; i < nthreads; i++) {
        args[i].id = i;
        threads[i] = pth_spawn(attr, travail_cpu, &args[i]);
        if (threads[i] == NULL) {
            fprintf(stderr, "Erreur: pth_spawn() thread %d\n", i);
            return 1;
        }
    }

    pth_attr_destroy(attr);

    /* Attendre la fin */
    for (int i = 0; i < nthreads; i++)
        pth_join(threads[i], NULL);

    double elapsed = get_time() - start;

    printf("═══ RÉSULTATS ═══\n");
    printf("  Temps réel     : %.2f s\n", elapsed);
    printf("\n");
    printf("  → Utilisez 'time ./bu_utilisateur' pour voir :\n");
    printf("     real ≈ user (les deux ≈ %.1fs → 1 seul cœur = PAS de SMP ❌)\n",
           elapsed);

    pth_kill();
    free(threads);
    free(args);
    return 0;
}
