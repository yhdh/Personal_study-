/* ================================================================
 * Q.7 : Stencil parallélisé avec des tâches OpenMP + dépendances
 *
 * Au lieu d'utiliser taskwait pour synchroniser, on utilise la
 * clause depend() pour exprimer les dépendances entre tâches :
 *
 * - Loops 1, 2, 3 (blur) :
 *     depend(in: mesh[0])            → lisent mesh
 *     depend(out: mesh_blur[region]) → écrivent dans mesh_blur
 *                                      (régions disjointes)
 *   → Elles peuvent s'exécuter en parallèle car les dépendances
 *     en sortie sont sur des adresses différentes.
 *
 * - Loop 4 (scale) :
 *     depend(in: mesh_blur[r1], mesh_blur[r2], mesh_blur[r3])
 *                                    → attend les 3 blurs
 *     depend(out: mesh[0])           → écrit dans mesh
 *   → Attend automatiquement les 3 tâches de blur grâce aux
 *     dépendances, pas besoin de taskwait !
 *
 * - À l'itération suivante, les tâches blur ont depend(in: mesh[0])
 *   → elles attendent automatiquement que le scale de l'itération
 *     précédente ait terminé d'écrire mesh.
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

	printf( "Running stencil (TASKDEP) on %d element(s) with seed %ld\n", N, seed ) ;

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

	/* Indices des 3 régions pour les dépendances */
	int r1 = 1 ;               /* début région blur gauche */
	int r2 ;                    /* début région copie milieu */
	int r3 ;                    /* début région blur droite */

	r2 = N / 10 - 1 ;
	r3 = (int)(N * 0.9 - 1) ;

	/* TIMER START */
	gettimeofday(&t1, NULL);

	/* Main Simulation Loop */
	#pragma omp parallel
	{
		#pragma omp single
		{
			do
			{
				int end1 = 1 ;
				int end3 = 1 ;

				/* ---- Blur Kernel (loop 1) - côté gauche ----
				 * Lit mesh, écrit mesh_blur[r1..r2-1]
				 * depend(in: mesh[0])         : attend que mesh soit prêt
				 * depend(out: mesh_blur[r1])   : signale écriture zone gauche
				 */
				#pragma omp task shared(mesh, mesh_blur, end1) firstprivate(N, threshold) \
					depend(in: mesh[0]) depend(out: mesh_blur[r1])
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

				/* ---- Blur Kernel (loop 2) - copie milieu ----
				 * Lit mesh, écrit mesh_blur[r2..r3-1]
				 * depend(in: mesh[0])         : attend que mesh soit prêt
				 * depend(out: mesh_blur[r2])   : signale écriture zone milieu
				 */
				#pragma omp task shared(mesh, mesh_blur) firstprivate(N) \
					depend(in: mesh[0]) depend(out: mesh_blur[r2])
				{
					int ii ;
					for ( ii = N/10-1 ; ii < (int)(N*0.9-1) ; ii++ )
					{
						mesh_blur[ii] = mesh[ii] ;
					}
				}

				/* ---- Blur Kernel (loop 3) - côté droit ----
				 * Lit mesh, écrit mesh_blur[r3..N-2]
				 * depend(in: mesh[0])         : attend que mesh soit prêt
				 * depend(out: mesh_blur[r3])   : signale écriture zone droite
				 */
				#pragma omp task shared(mesh, mesh_blur, end3) firstprivate(N, threshold) \
					depend(in: mesh[0]) depend(out: mesh_blur[r3])
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

				/* ---- Scale Kernel ----
				 * Lit mesh_blur (toutes les zones), écrit mesh
				 * depend(in: mesh_blur[r1], mesh_blur[r2], mesh_blur[r3])
				 *   → attend les 3 tâches de blur automatiquement !
				 * depend(out: mesh[0])
				 *   → les blurs de l'itération SUIVANTE attendront
				 *     que le scale soit terminé.
				 */
				#pragma omp task shared(mesh, mesh_blur) firstprivate(N) \
					depend(in: mesh_blur[r1], mesh_blur[r2], mesh_blur[r3]) \
					depend(out: mesh[0])
				{
					int ii ;
					for ( ii = 0 ; ii < N ; ii++ )
					{
						mesh[ii] = mesh_blur[ii] * 3.3 / 5.4 ;
					}
				}

				/* On a besoin de taskwait ici pour lire end1 et end3
				 * qui sont modifiés par les tâches de blur.
				 * Les dépendances gèrent l'ordonnancement blur→scale,
				 * mais le thread single doit attendre avant de tester 'end'. */
				#pragma omp taskwait

				end = end1 && end3 ;
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
