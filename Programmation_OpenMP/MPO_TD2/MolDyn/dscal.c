
/*
 *  Scales an array
 *
 *  Q5 : Parallélisé avec #pragma omp for.
 *  Appelée depuis la région parallèle de main.c.
 *  Chaque itération est indépendante (sx[i] *= sa).
 *  Pour incx != 1, on calcule l'index directement : j = i * incx.
 *
 *  Version séquentielle originale :
 *  if (incx == 1) { for (i=0;i<n;i++) sx[i]*=sa; }
 *  else { j=0; for (i=0;i<n;i++) { sx[j]*=sa; j+=incx; } }
 */
  void
  dscal(int n,double sa,double sx[], int incx){
    int i;

    if (incx == 1) {
      #pragma omp for schedule(static)
      for (i=0; i<n; i++)
        sx[i] *= sa;
    } else {
      #pragma omp for schedule(static)
      for (i=0; i<n; i++)
        sx[i * incx] *= sa;
    }
  }
