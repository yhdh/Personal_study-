/*
 *  Scales an array
 */
// TODO parallélisation
  void dscal(int n,double sa,double sx[], int incx){
    int i,j;

    if (incx == 1) {
      #pragma omp for schedule(runtime)
      for (i=0; i<n; i++)
        sx[i] *= sa;
    } else {
      j = 0;
      #pragma omp for schedule(runtime)
      for (i=0; i<n; i++) {
        sx[j] *= sa;
        j += incx;
      }
    }
  }