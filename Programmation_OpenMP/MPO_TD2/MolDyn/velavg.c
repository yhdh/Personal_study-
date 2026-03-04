#include <math.h>

/*
 *  Compute average velocity
 *
 *  Q5 : Parallélisé avec #pragma omp for.
 *  Appelée depuis la région parallèle de main.c.
 *  Même technique que mkekin : partial_vel et partial_count (privés)
 *  puis omp atomic vers les variables static (partagées).
 *
 *  Version séquentielle originale :
 *  count=0.0;
 *  for (i=0; i<npart*3; i+=3) {
 *    sq=sqrt(vh[i]*vh[i]+vh[i+1]*vh[i+1]+vh[i+2]*vh[i+2]);
 *    if (sq>vaverh) count++;  vel+=sq;
 *  }
 */
  double
  velavg(int npart, double vh[], double vaver, double h){
    int i;
    double vaverh=vaver*h;
    double partial_vel=0.0;
    double partial_count=0.0;
    double sq;
    extern double count;
    static double total_vel;
    static double total_count;

    #pragma omp single
    {
      total_vel=0.0;
      total_count=0.0;
    }
    /* barrière implicite */

    #pragma omp for schedule(static)
    for (i=0; i<npart*3; i+=3){
      sq=sqrt(vh[i]*vh[i]+vh[i+1]*vh[i+1]+vh[i+2]*vh[i+2]);
      if (sq>vaverh) partial_count+=1.0;
      partial_vel+=sq;
    }
    /* barrière implicite */

    #pragma omp atomic
    total_vel += partial_vel;

    #pragma omp atomic
    total_count += partial_count;

    #pragma omp barrier  /* tous les threads ont ajouté leur part */

    /* Un seul thread met à jour la variable globale count */
    #pragma omp single
    count=total_count;
    /* barrière implicite */

    return(total_vel/h);
  }
