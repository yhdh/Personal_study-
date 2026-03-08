/* ================================================================
 * Q.7 : Stencil parallélisé avec des tâches OpenMP (+ taskwait)
 *
 * Structure du parallélisme :
 * - Les 3 boucles de blur (loop 1, 2, 3) opèrent sur des régions
 *   disjointes de mesh_blur et lisent toutes dans mesh
 *   → elles peuvent s'exécuter en PARALLÈLE (3 tâches).
 * - La boucle scale (loop 4) lit mesh_blur et écrit mesh
 *   → elle DÉPEND de la fin des 3 boucles de blur.
 * - Un taskwait sépare les deux phases.
 * ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

int main( int argc, char ** argv )
{
	double * mesh ;
	double * mesh_blur ;
	int N ;
	long int seed ;
	double threshold = 0.01 ;
	int i ;
	int end ;
	int niter = 0 ;
	struct timeval t1, t2;
	double duration ;

	/* Check number of arguments */
	if ( argc < 3 )
	{
		fprintf( stderr, "Usage: %s N seed\n", argv[0] ) ;
		return 1 ;
	}

	/* Get the number of elements */
	N = atoi( argv[1] ) ;
	if ( N <= 0 )
	{
		fprintf( stderr, "ERROR: number of elements N (%d) should be > 0\n", N ) ;
		return 1 ;
	}

	/* Get the seed for random-number generator */
	seed = atol( argv[2] ) ;

	/* Initialize the random-number generator */
	srand48( seed ) ;

	printf( "Running stencil (TASK) on %d element(s) with seed %ld\n", N, seed ) ;

	/* Memory allocation */
	mesh = (double *) malloc(N * sizeof( double ) ) ;
	if ( mesh == NULL )
	{
		fprintf( stderr, "ERROR: unable to allocate mesh with %d elements\n", N ) ;
		return 1 ;
	}
	mesh_blur = (double *) malloc(N * sizeof( double ) ) ;
	if ( mesh_blur == NULL )
	{
		fprintf( stderr, "ERROR: unable to allocate mesh_blur with %d elements\n", N ) ;
		return 1 ;
	}

	/* Mesh initialization (w/ random value) */
	for ( i = 0 ; i < N ; i++ )
	{
		mesh[i] = drand48() ;
	}

	/* TIMER START */
	gettimeofday(&t1, NULL);

	/* Main Simulation Loop */
	#pragma omp parallel
	{
		#pragma omp single
		{
			do
			{
				/*
				 * end1 et end3 sont des variables locales pour éviter
				 * une data race sur 'end' entre les tâches 1 et 3.
				 */
				int end1 = 1 ;
				int end3 = 1 ;

				/* ---- Phase 1 : Blur (3 tâches indépendantes) ---- */

				/* Blur Kernel (loop 1) - côté gauche */
				#pragma omp task shared(mesh, mesh_blur, end1) firstprivate(N, threshold)
				{
					int ii ;
					for ( ii = 1 ; ii < N/10-1 ; ii++ )
					{
						double diff ;
						mesh_blur[ii] = (mesh[ii-1] + mesh[ii] + mesh[ii+1]) / 3 ;
						diff = mesh_blur[ii] - mesh[ii] ;
						if ( diff < 0 ) diff = -diff ;
						if ( diff > threshold ) end1 = 0 ;
					}
				}

				/* Blur Kernel (loop 2) - copie milieu */
				#pragma omp task shared(mesh, mesh_blur) firstprivate(N)
				{
					int ii ;
					for ( ii = N/10-1 ; ii < (int)(N*0.9-1) ; ii++ )
					{
						mesh_blur[ii] = mesh[ii] ;
					}
				}

				/* Blur Kernel (loop 3) - côté droit */
				#pragma omp task shared(mesh, mesh_blur, end3) firstprivate(N, threshold)
				{
					int ii ;
					for ( ii = (int)(N*0.9-1) ; ii < N - 1 ; ii++ )
					{
						double diff ;
						mesh_blur[ii] = (mesh[ii-1] + mesh[ii] + mesh[ii+1]) / 3 ;
						diff = mesh_blur[ii] - mesh[ii] ;
						if ( diff < 0 ) diff = -diff ;
						if ( diff > threshold ) end3 = 0 ;
					}
				}

				/* Synchronisation : on attend la fin des 3 boucles de blur
				 * avant de lancer le scale kernel */
				#pragma omp taskwait

				/* Combiner les résultats de convergence */
				end = end1 && end3 ;

				/* ---- Phase 2 : Scale Kernel ---- */
				#pragma omp task shared(mesh, mesh_blur) firstprivate(N)
				{
					int ii ;
					for ( ii = 0 ; ii < N ; ii++ )
					{
						mesh[ii] = mesh_blur[ii] * 3.3 / 5.4 ;
					}
				}

				/* Synchronisation : on attend la fin du scale kernel
				 * avant de passer à l'itération suivante (qui lit mesh) */
				#pragma omp taskwait

				niter++ ;
			}
			while( !end ) ;
		} /* fin single */
	} /* fin parallel */

	/* TIMER STOP */
	gettimeofday(&t2, NULL);

	duration = (t2.tv_sec - t1.tv_sec) + ((t2.tv_usec - t1.tv_usec) / 1e6) ;

	printf( "DONE w/ %d iter in %lf s\n", niter, duration ) ;

	/* Output first elements of the mesh */
	printf( "\tMESH " ) ;
	for ( i = 0 ; i < 10 ; i++ )
	{
		printf( "%.2f ", mesh[i] ) ;
	}
	printf( "\n" ) ;

	free( mesh ) ;
	free( mesh_blur ) ;

	return 0 ;
}
