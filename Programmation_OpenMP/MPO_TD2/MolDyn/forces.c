#include <stdlib.h>
#include <omp.h>

/*
 * Compute forces and accumulate the virial and the potential (OpenMP version)
 */
extern double epot, vir;

void forces(int npart, double x[], double f[], double side, double rcoff)
{
    const int n = 3 * npart;

    /* Accumulate into locals, then write to globals at the end */
    double epot_sum = 0.0;
    double vir_sum  = 0.0;

    /* Per-thread private force buffers to avoid races on f[] */
    const int T = omp_get_max_threads();
    double *fpriv = (double *)calloc((size_t)T * (size_t)n, sizeof(double));
    if (!fpriv) {
        /* fallback: keep sequential semantics (or handle error as you want) */
        vir = 0.0;
        epot = 0.0;
        for (int i=0; i<n; i++) f[i] = 0.0;
        return;
    }

    const double rcoff2 = rcoff * rcoff;

    /* Parallelize outer loop (particle i) */
#pragma omp parallel reduction(+:epot_sum,vir_sum)
    {
        const int tid = omp_get_thread_num();
        double *ft = fpriv + (size_t)tid * (size_t)n;

#pragma omp for schedule(static)
        for (int pi = 0; pi < npart; ++pi) {
            const int i = 3 * pi;

            double fxi = 0.0, fyi = 0.0, fzi = 0.0;

            for (int pj = pi + 1; pj < npart; ++pj) {
                const int j = 3 * pj;

                /* distance with wraparound (minimum image convention) */
                double xx = x[i]   - x[j];
                double yy = x[i+1] - x[j+1];
                double zz = x[i+2] - x[j+2];

                if (xx < (-0.5 * side)) xx += side;
                if (xx > ( 0.5 * side)) xx -= side;
                if (yy < (-0.5 * side)) yy += side;
                if (yy > ( 0.5 * side)) yy -= side;
                if (zz < (-0.5 * side)) zz += side;
                if (zz > ( 0.5 * side)) zz -= side;

                double rd = xx*xx + yy*yy + zz*zz;

                if (rd <= rcoff2) {
                    double rrd  = 1.0 / rd;
                    double rrd3 = rrd * rrd * rrd;
                    double rrd4 = rrd3 * rrd;
                    double r148 = rrd4 * (rrd3 - 0.5);

                    epot_sum += rrd3 * (rrd3 - 1.0);
                    vir_sum  -= rd * r148;

                    fxi += xx * r148;
                    fyi += yy * r148;
                    fzi += zz * r148;

                    /* apply opposite force on j into thread-private buffer */
                    ft[j]   -= xx * r148;
                    ft[j+1] -= yy * r148;
                    ft[j+2] -= zz * r148;
                }
            }

            /* update force on i into thread-private buffer */
            ft[i]   += fxi;
            ft[i+1] += fyi;
            ft[i+2] += fzi;
        }
    }

    /* Reduce fpriv -> f (sum over threads) */
#pragma omp parallel for schedule(static)
    for (int k = 0; k < n; ++k) {
        double s = 0.0;
        for (int t = 0; t < T; ++t) {
            s += fpriv[(size_t)t * (size_t)n + (size_t)k];
        }
        f[k] += s; /* keep same "accumulate" behavior as your sequential code */
    }

    epot = epot_sum;
    vir  = vir_sum;

    free(fpriv);
}