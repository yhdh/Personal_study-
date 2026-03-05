
/*
 *  Move particles
 *
 *  Q5 : Parallélisé avec #pragma omp for.
 *  Chaque itération est indépendante (x[i], vh[i], f[i] ne dépendent
 *  que de l'indice i) → parallélisation directe, pas de réduction.
 *  Appelée depuis la région parallèle de main.c.
 *
 *  Version séquentielle originale :
 *  for (i=0; i<n3; i++) {
 *    x[i] += vh[i]+f[i];
 *    if (x[i] < 0.0)  x[i] += side;
 *    if (x[i] > side) x[i] -= side;
 *    vh[i] += f[i];
 *    f[i] = 0.0;
 *  }
 */
  void
  domove(int n3, double x[], double vh[], double f[], double side){
    int i;

    #pragma omp for schedule(static)
    for (i=0; i<n3; i++) {
      x[i] += vh[i]+f[i];
  /*
   *  Periodic boundary conditions
   */
      if (x[i] < 0.0)  x[i] += side;
      if (x[i] > side) x[i] -= side;
  /*
   *  Partial velocity updates
   */
      vh[i] += f[i];
  /*
   *  Initialise forces for the next iteration
   */
      f[i] = 0.0;
    }
  }
