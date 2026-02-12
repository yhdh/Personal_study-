#include <math.h>

extern double count;
double vel;

/*
 *  Compute average velocity
 */
// TODO parallélisation
  double
  velavg(int npart, double vh[], double vaver, double h){
    int i;
    double vaverh=vaver*h;
    
    #pragma omp single
    {
      vel=0.0;
      count=0.0;
    }

    double sq;

    #pragma omp for schedule(runtime) reduction(+:vel,count)
    for (i=0; i<npart*3; i+=3){
      sq=sqrt(vh[i]*vh[i]+vh[i+1]*vh[i+1]+vh[i+2]*vh[i+2]);
      if (sq>vaverh) count++;
      vel+=sq;
    }
    #pragma omp single
    {
      vel/=h;
    }

    return(vel);
  }