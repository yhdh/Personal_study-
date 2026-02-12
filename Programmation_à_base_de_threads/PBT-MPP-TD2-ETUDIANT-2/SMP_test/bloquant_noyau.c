/*
 * bloquant_noyau.c — Test appels bloquants avec Pthread (niveau noyau)
 *
 * On crée N threads. Chaque thread fait un sleep() (appel bloquant)
 * de SLEEP_TIME secondes. Avec Pthread, seul le thread appelant est
 * bloqué → les autres continuent.
 *
 * Si parallélisme réel : temps total ≈ SLEEP_TIME (tous dorment en même temps)
 * Si pas de parallélisme : temps total ≈ N × SLEEP_TIME (séquentiel)
 *
 * Compiler :
 *   gcc -O2 -pthread bloquant_noyau.c -o bloquant_noyau
 *
 * Exécuter :
 *   time ./bloquant_noyau 4
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

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

    printf("  [Thread %d] Début sleep(%d)...\n", ta->id, SLEEP_TIME);
    sleep(SLEEP_TIME);  /* appel système BLOQUANT */
    double elapsed = get_time() - start;
    printf("  [Thread %d] Réveil après %.2f s\n", ta->id, elapsed);

    return NULL;
}

int main(int argc, char **argv)
{
    int nthreads = (argc > 1) ? atoi(argv[1]) : 4;

    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  TEST APPELS BLOQUANTS — NOYAU (Pthread)            ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Threads       : %-4d                                ║\n", nthreads);
    printf("║  Sleep/thread  : %d s                                ║\n", SLEEP_TIME);
    printf("║  Temps attendu : ~%d s (si bloquants gérés)         ║\n", SLEEP_TIME);
    printf("║  Temps pire cas: ~%d s (si tout bloqué)             ║\n", SLEEP_TIME * nthreads);
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    pthread_t *threads = malloc(nthreads * sizeof(pthread_t));
    thread_arg_t *args = malloc(nthreads * sizeof(thread_arg_t));

    double start = get_time();

    /* Créer les threads */
    for (int i = 0; i < nthreads; i++) {
        args[i].id = i;
        pthread_create(&threads[i], NULL, thread_bloquant, &args[i]);
    }

    /* Attendre la fin */
    for (int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

    double elapsed = get_time() - start;

    printf("\n═══ RÉSULTATS ═══\n");
    printf("  Threads        : %d\n", nthreads);
    printf("  Temps total    : %.2f s\n", elapsed);
    printf("  Temps attendu  : %d s (si appels bloquants gérés correctement)\n", SLEEP_TIME);
    printf("  Temps pire cas : %d s (si tout bloqué séquentiellement)\n", SLEEP_TIME * nthreads);

    if (elapsed < SLEEP_TIME * 1.5)
        printf("  → Résultat : ✅ Appels bloquants bien gérés (seul le thread est bloqué)\n");
    else
        printf("  → Résultat : ❌ Appels bloquants MAL gérés (tout le processus bloqué)\n");

    free(threads);
    free(args);
    return 0;
}
