/*
 *  Compute forces and accumulate the virial and the potential
 *
 *  ===================================================================
 *  Q6 : Version avec TABLEAU TEMPORAIRE indexé par numéro de thread
 *  ===================================================================
 *
 *  Problème des versions précédentes :
 *    Q2 utilisait #pragma omp critical pour accumuler f_local → f[].
 *    Les instructions atomic/critical constituent un GOULOT D'ÉTRANGLEMENT
 *    car elles sérialisent l'accès : un seul thread à la fois peut
 *    exécuter la section critique → perte de parallélisme.
 *
 *  Solution Q6 : Tableau 2D f_all[nthreads][n3]
 *    - Chaque thread écrit dans sa propre tranche f_all[tid * n3 + k]
 *    - AUCUNE synchronisation nécessaire pendant le calcul !
 *    - L'accumulation finale est elle-même parallélisée avec #pragma omp for
 *    - Le tableau est alloué une seule fois (variable static) et réutilisé.
 *
 *  Note : Cette version est conçue pour être appelée depuis une
 *  région parallèle déjà ouverte dans main.c (Q4/Q5).
 *  ===================================================================
 *
 *  ===== ANCIENNE VERSION Q2 (avec critical) — pour référence =======
 *
 *  void forces(int npart, double x[], double f[], double side, double rcoff){
 *    int i;
 *    int n3 = npart * 3;
 *    vir = 0.0;
 *    epot = 0.0;
 *
 *    #pragma omp parallel default(none)
 *      shared(x, f, side, rcoff, npart, n3)
 *      reduction(+:epot, vir)
 *    {
 *      double *f_local = (double *)calloc(n3, sizeof(double));
 *
 *      #pragma omp for schedule(runtime)
 *      for (i = 0; i < n3; i += 3) {
 *        double fxi=0.0, fyi=0.0, fzi=0.0;
 *        int j;
 *        for (j = i+3; j < n3; j += 3) {
 *          ... même calcul ...
 *          if (rd <= rcoff*rcoff) {
 *            ... épot, vir, fxi, fyi, fzi ...
 *            f_local[j]   -= xx * r148;  // PAS f[j] !
 *            f_local[j+1] -= yy * r148;
 *            f_local[j+2] -= zz * r148;
 *          }
 *        }
 *        f_local[i] += fxi; f_local[i+1] += fyi; f_local[i+2] += fzi;
 *      }
 *
 *      // GOULOT D'ÉTRANGLEMENT :
 *      #pragma omp critical        ← sérialise l'accumulation !
 *      {
 *        for (k = 0; k < n3; k++)
 *          f[k] += f_local[k];
 *      }
 *      free(f_local);
 *    }
 *  }
 *
 *  ===== FIN ANCIENNE VERSION Q2 ===================================
 */
  #include <omp.h>
  #include <string.h>
  #include <stdlib.h>

  extern double epot, vir;

  void
  forces(int npart, double x[], double f[], double side, double rcoff){

    int   i;
    int   n3 = npart * 3;
    int   nthreads = omp_get_num_threads();
    int   tid      = omp_get_thread_num();

    /*
     * Tableau partagé : f_all[tid * n3 + k]
     * static pour ne pas réallouer à chaque appel (forces est appelée
     * à chaque pas de temps).
     */
    static double *f_all = NULL;
    static int f_all_size = 0;

    /* Un seul thread initialise les variables globales et (ré)alloue si nécessaire */
    #pragma omp single
    {
      vir  = 0.0;
      epot = 0.0;
      int needed = nthreads * n3;
      if (f_all_size < needed) {
        free(f_all);
        f_all = (double *)malloc(needed * sizeof(double));
        f_all_size = needed;
      }
    }
    /* Barrière implicite après single → tous les threads voient f_all */

    /* Chaque thread initialise sa propre tranche à zéro (en parallèle) */
    double *my_f = &f_all[tid * n3];
    memset(my_f, 0, n3 * sizeof(double));

    #pragma omp barrier  /* S'assurer que toutes les tranches sont initialisées */

    #pragma omp for schedule(runtime) reduction(+:epot, vir)
    for (i = 0; i < n3; i += 3) {

      double fxi = 0.0;
      double fyi = 0.0;
      double fzi = 0.0;

      int j;
      for (j = i + 3; j < n3; j += 3) {

        double xx = x[i]   - x[j];
        double yy = x[i+1] - x[j+1];
        double zz = x[i+2] - x[j+2];

        if (xx < (-0.5*side)) xx += side;
        if (xx > ( 0.5*side)) xx -= side;
        if (yy < (-0.5*side)) yy += side;
        if (yy > ( 0.5*side)) yy -= side;
        if (zz < (-0.5*side)) zz += side;
        if (zz > ( 0.5*side)) zz -= side;

        double rd = xx*xx + yy*yy + zz*zz;

        if (rd <= rcoff*rcoff) {

          double rrd  = 1.0/rd;
          double rrd3 = rrd*rrd*rrd;
          double rrd4 = rrd3*rrd;
          double r148 = rrd4*(rrd3 - 0.5);

          epot += rrd3*(rrd3 - 1.0);
          vir  -= rd * r148;

          fxi += xx * r148;
          fyi += yy * r148;
          fzi += zz * r148;

          /*
           * Écriture dans la tranche du thread (my_f = f_all + tid*n3)
           * → PAS de conflit d'accès, PAS de critical, PAS de atomic !
           */
          my_f[j]   -= xx * r148;
          my_f[j+1] -= yy * r148;
          my_f[j+2] -= zz * r148;
        }
      }

      my_f[i]   += fxi;
      my_f[i+1] += fyi;
      my_f[i+2] += fzi;
    }

    /*
     * ACCUMULATION FINALE — elle-même parallélisée !
     * Chaque thread accumule une portion du tableau f[].
     * PAS de critical, PAS de atomic.
     *
     * Complexité : O(n3 * nthreads / nthreads) = O(n3)
     * (alors que la version critical était O(n3 * nthreads) séquentiel)
     */
    #pragma omp for schedule(static)
    for (i = 0; i < n3; i++) {
      int t;
      for (t = 0; t < nthreads; t++) {
        f[i] += f_all[t * n3 + i];
      }
    }
    /* Barrière implicite après omp for → f[] est complet */
  }
