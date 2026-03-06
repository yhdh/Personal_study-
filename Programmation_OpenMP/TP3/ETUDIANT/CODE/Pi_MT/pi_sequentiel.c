#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <omp.h>

int N_TASKS_PER_THREAD = 1000;
int N_TRIALS_PER_TASK  = 10000;

int main(int argc, char** argv)
{
  uint64_t count = 0;
  double pi = 0.;
  double t0, t1;

  int nthreads = 1;

  t0 = omp_get_wtime();

  #pragma omp parallel
  {
    
    #pragma omp single
    {
      nthreads = omp_get_num_threads();
      fprintf(stdout, "Running with %d thread(s)\n", nthreads);
      fprintf(stdout, "Tasks per thread : %d\n", N_TASKS_PER_THREAD);
      fprintf(stdout, "Trials per task  : %d\n", N_TRIALS_PER_TASK);

      for (int t = 0; t < nthreads * N_TASKS_PER_THREAD; t++)
      {
        #pragma omp task firstprivate(t)
        {
          
          unsigned int seed = (unsigned int)(t * 31 + 2020);
          uint64_t local_count = 0;
          double x, y;

          for (int k = 0; k < N_TRIALS_PER_TASK; k++)
          {
            x = rand_r(&seed) / (double)RAND_MAX;
            y = rand_r(&seed) / (double)RAND_MAX;
            local_count += (((x * x) + (y * y)) <= 1.0);
          }

          #pragma omp atomic
          count += local_count;
        }
      }
    } 
  } 

  t1 = omp_get_wtime();

  uint64_t n_total = (uint64_t)nthreads * N_TASKS_PER_THREAD * N_TRIALS_PER_TASK;
  fprintf(stdout, "%" PRIu64 " of %" PRIu64 " throws are in the circle !\n", count, n_total);
  pi = (count * 4.0) / (double)n_total;
  fprintf(stdout, "Pi ~= %lf\n", pi);
  fprintf(stdout, "Time: %f s\n", t1 - t0);

  return 0;
}
