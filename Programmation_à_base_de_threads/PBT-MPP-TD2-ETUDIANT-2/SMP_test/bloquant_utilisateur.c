/*
 * bloquant_utilisateur.c — Test appels bloquants avec GnuPth (niveau utilisateur)
 *
 * On crée N threads. Chaque thread fait un sleep() (appel bloquant)
 * de SLEEP_TIME secondes.
 *
 * GnuPth intercepte les appels bloquants via des wrappers et les
 * remplace par des versions non-bloquantes + select()/poll().
 * Ainsi, quand un thread dort, GnuPth peut ordonnancer un autre thread.
 *
 * Si bien géré : temps total ≈ SLEEP_TIME (tous dorment "en parallèle")
 * Si mal géré : temps total ≈ N × SLEEP_TIME (séquentiel)
 *
 * Compiler :
 *   gcc -O2 -I../pth-2.0.7 -L../pth-2.0.7/.libs bloquant_utilisateur.c \
 *       -lpth -o bloquant_utilisateur
 *
 * Exécuter :
 *   export DYLD_LIBRARY_PATH=../pth-2.0.7/.libs:$DYLD_LIBRARY_PATH
 *   time ./bloquant_utilisateur 4
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "pth.h"

#define SLEEP_TIME 2  /* secondes de blocage par thread */

typedef struct {
    int id;
} thread_arg_t;

double get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

void *thread_bloquant(void *arg)
{
    thread_arg_t *ta = (thread_arg_t *)arg;
    double start = get_time();

    printf("  [Thread %d] Début pth_sleep(%d)...\n", ta->id, SLEEP_TIME);
    pth_sleep(SLEEP_TIME);  /* appel bloquant GnuPth (wrapper de sleep) */
    double elapsed = get_time() - start;
    printf("  [Thread %d] Réveil après %.2f s\n", ta->id, elapsed);

    return NULL;
}

int main(int argc, char **argv)
{
    int nthreads = (argc > 1) ? atoi(argv[1]) : 4;

    /* Initialiser GnuPth */
    if (!pth_init()) {
        fprintf(stderr, "Erreur: pth_init() a échoué\n");
        return 1;
    }

    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  TEST APPELS BLOQUANTS — UTILISATEUR (GnuPth)       ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Threads       : %-4d                                ║\n", nthreads);
    printf("║  Sleep/thread  : %d s                                ║\n", SLEEP_TIME);
    printf("║  Temps attendu : ~%d s (si bloquants gérés)         ║\n", SLEEP_TIME);
    printf("║  Temps pire cas: ~%d s (si tout bloqué)             ║\n", SLEEP_TIME * nthreads);
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    pth_t *threads = malloc(nthreads * sizeof(pth_t));
    thread_arg_t *args = malloc(nthreads * sizeof(thread_arg_t));

    pth_attr_t attr = pth_attr_new();
    pth_attr_set(attr, PTH_ATTR_JOINABLE, TRUE);

    double start = get_time();

    /* Créer les threads */
    for (int i = 0; i < nthreads; i++) {
        args[i].id = i;
        threads[i] = pth_spawn(attr, thread_bloquant, &args[i]);
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

    printf("\n═══ RÉSULTATS ═══\n");
    printf("  Threads        : %d\n", nthreads);
    printf("  Temps total    : %.2f s\n", elapsed);
    printf("  Temps attendu  : %d s (si appels bloquants gérés correctement)\n", SLEEP_TIME);
    printf("  Temps pire cas : %d s (si tout bloqué séquentiellement)\n", SLEEP_TIME * nthreads);

    if (elapsed < SLEEP_TIME * 1.5)
        printf("  → Résultat : ✅ Appels bloquants bien gérés (GnuPth intercepte sleep)\n");
    else
        printf("  → Résultat : ❌ Appels bloquants MAL gérés (tout le processus bloqué)\n");

    pth_kill();
    free(threads);
    free(args);
    return 0;
}
