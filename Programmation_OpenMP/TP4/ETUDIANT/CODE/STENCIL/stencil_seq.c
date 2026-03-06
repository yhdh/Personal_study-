#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


int main( int argc, char ** argv )
{
	double * mesh ; 
	double * mesh_blur ;
	int N ;
	long int seed ;
	double threshold = 0.01 ;
	int i ;
	int end ;
	int niter ;
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

	printf( "Running stencil on %d element(s) with seed %ld\n", N, seed ) ;

	/* Memory allocation */
	mesh = (double *) malloc(N * sizeof( double ) ) ;
	if ( mesh == NULL )
	{
		fprintf( stderr, "ERROR: unable to allocate mesh with %d elements\n", N ) ;
		return 1 ;
	}
	mesh_blur = (double *) malloc(N * sizeof( double ) ) ;
	if ( mesh == NULL )
	{
		fprintf( stderr, "ERROR: unable to allocate mesh_blur with %d elements\n", N ) ;
		return 1 ;
	}


	/* Mesh initialization (w/ random value)*/
	for ( i = 0 ; i < N ; i++ )
	{
		mesh[i] = drand48() ;
	}

	/* TIMER START */
	gettimeofday(&t1, NULL);

	/* Main Simulation Loop */
	do
	{
		end = 1 ;

		/* Blur Kernel (loop 1) */
		for ( i = 1 ; i < N/10-1 ; i++ )
		{
			double diff ;

			mesh_blur[i] = (mesh[i-1] + mesh[i] + mesh[i+1]) / 3 ;

			diff = mesh_blur[i] - mesh[i] ;
			if ( diff < 0 ) diff = -diff ;
			if ( diff > threshold ) end = 0 ;
		}

		/* Blur Kernel (loop 2) */
		for ( i = N/10-1  ; i < (int)(N*0.9-1) ; i++ )
		{
			mesh_blur[i] = mesh[i] ;
		}

		/* Blur Kernel (loop 3) */
		for ( i = (int)(N*0.9-1) ; i < N -1 ; i++ )
		{
			double diff ;
			mesh_blur[i] = (mesh[i-1] + mesh[i] + mesh[i+1]) / 3 ;

			diff = mesh_blur[i] - mesh[i] ;
			if ( diff < 0 ) diff = -diff ;
			if ( diff > threshold ) end = 0 ;
		}


		/* Scale Kernel */
		for ( i = 0 ; i < N ; i++ )
		{
			mesh[i] = mesh_blur[i] * 3.3 / 5.4 ;
		}

		niter++ ;
	}
	while( !end ) ;

	/* TIMER STOP */
	gettimeofday(&t2, NULL);

	duration = (t2.tv_sec -t1.tv_sec)+((t2.tv_usec-t1.tv_usec)/1e6);

	printf( "DONE w/ %d iter in %lf s\n", niter, duration ) ;

	/* Output first elements of the mesh */
	printf( "\tMESH " ) ;
	for ( i = 0 ; i < 10 ; i++ )
	{
		printf( "%.2f ", mesh[i] ) ;
	}
	printf( "\n" ) ;

	return 0 ;
}
