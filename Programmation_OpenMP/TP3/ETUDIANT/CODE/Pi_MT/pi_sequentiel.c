#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

int nthread = 10; 
int N_TASKS_PER_THREAD = 1000; 
int N_TRIALS_PER_TASK = 10000; 

int main(int argc, char** argv)
{
  uint64_t n_test = 10E7;
  uint64_t i;
  uint64_t count = 0;
  double x = 0., y = 0.;
  double pi = 0.;

  srand(2020);
#pragma omp parallel
{
    for (int j = 0; j < N_TASKS_PER_THREAD; j++){
#pragma omp task
{    
      uint64_t local_count = 0;
      for(int k = 0; k < N_TRIALS_PER_TASK; k++){
            x = rand() / (double)RAND_MAX;
            y = rand() / (double)RAND_MAX;

            local_count += (((x * x) + (y * y)) <= 1);
      }
#pragma omp atomic
      count += local_count;    
}
    }
}

 /* for(i = 0; i < n_test; ++i)
  {
    x = rand() / (double)RAND_MAX;
    y = rand() / (double)RAND_MAX;

    count += (((x * x) + (y * y)) <= 1);
  }*/

  fprintf(stdout, "%ld of %ld throws are in the circle !\n", count, n_test);
  pi = (count * 4) / (double)n_test;
  fprintf(stdout, "Pi ~= %lf\n", pi);

  return 0;
}
