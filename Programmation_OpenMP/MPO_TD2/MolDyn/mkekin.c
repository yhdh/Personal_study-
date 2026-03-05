#include <stdio.h>
/*
 *  Scale forces, update velocities and compute K.E.
 *
 *  Q5 : Parallélisé avec #pragma omp for.
 *  Appelée depuis la région parallèle de main.c.
 *  Chaque itération modifie f[i] et vh[i] de manière indépendante.
 *
 *  Note : On ne peut PAS utiliser reduction(+:sum) sur une variable
 *  locale car elle est implicitement privée dans le contexte parallèle
 *  extérieur (erreur GCC : "reduction variable is private in outer context").
 *  Solution : chaque thread accumule dans partial_sum (privé) puis
 *  ajoute à total_sum (static/partagé) via #pragma omp atomic.
 *  L'atomic n'est exécuté qu'UNE FOIS par thread (pas par itération)
 *  → overhead négligeable.
 */
  double
  mkekin(int npart, double f[], double vh[], double hsq2, double hsq){
    int i;
    double partial_sum = 0.0;
    static double total_sum;

    #pragma omp single
    total_sum = 0.0;
    /* barrière implicite */

    #pragma omp for schedule(static)
    for (i=0; i<3*npart; i++) {
      f[i]*=hsq2;
      vh[i]+=f[i];
      partial_sum+=vh[i]*vh[i];
    }
    /* barrière implicite */

    #pragma omp atomic
    total_sum += partial_sum;

    #pragma omp barrier  /* tous les threads ont ajouté leur part */

    return(total_sum/hsq);
  }
