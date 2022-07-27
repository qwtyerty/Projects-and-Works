#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "common.h"
#include <vector>
#include "omp.h"


/**
  Parallel Implementation of particule calculations
    Simulates the interaction of n particles with a correctness check
    Uses binning to reduces the number of checks required and approaches O(N)
      rather than being O(N^2)
    Each thread handles a set of particles which calculates a set of bins to calculate only
      the necessary particles who are close enough to a target particle to have any bearing on the 
      forces between each other.
**/

//
//  benchmarking program
//
int main( int argc, char **argv )
{    
    int navg,nabsavg=0, numThreads;
    double davg,dmin, absmin=1.0, absavg=0.0;

    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        printf( "-s <filename> to specify a summary file name\n" );
        printf( "-no turns off all correctness checks and particle output\n");
        return 0;
    }
    
    int n = read_int( argc, argv, "-n", 1000 );

    char *savename = read_string( argc, argv, "-o", NULL );
    char *sumname = read_string( argc, argv, "-s", NULL );
    
    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
    FILE *fsum = sumname ? fopen ( sumname, "a" ) : NULL;

    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );
    set_size( n );
    init_particles( n, particles );
    
    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );


   
    for( int step = 0; step < NSTEPS; step++ )
    {
	navg = 0;
        davg = 0.0;
	dmin = 1.0;
        
	//Get some variables we will use for binning
    	double size = sqrt(n * 0.0005);

    	// we are gonna create bins as a numCell by numCell matrix and allocate each bin
    	int numCells = (int) ceil(size / 0.01);
    	std::vector<particle_t*> partBins[numCells][numCells];
    	for(int i = 0; i < numCells; ++i)
	    	for(int j = 0; j < numCells; ++j)
		    partBins[i][j] = std::vector<particle_t*>();

    	double cellSize = size / numCells;

    	// We can now fill this matrix with particles
    	for(int i = 0; i < n; ++i)
	{
	   int xBin = floor(particles[i].x / cellSize);
    	   int yBin = floor(particles[i].y / cellSize);

    	   //Add this particle to (xBin, yBin)
    	   partBins[yBin][xBin].push_back(&particles[i]);
	}

	//
        //  compute forces
        //

	//Start Parallel Section
#pragma omp parallel
	{
	//Figure out how many threads we have for later
	numThreads = omp_get_num_threads();
#pragma omp for collapse(2) reduction(+:navg) reduction(+:davg) firstprivate(dmin) schedule(dynamic)
        for(int r = 0; r < numCells; ++r)
	{
	  for(int c = 0; c < numCells; ++c)
	  {
	    // Get the number of threads
	   
	    // Storage for the current bin and neighboring bins
	    std::vector<std::vector<particle_t*>> nearBins;

	    //Store the neighbors which is 3x3 around (c, r) 
	    for(int i = r - 1; i <= r + 1; ++i)
	      for(int j = c - 1; j <= c + 1; ++j)
		if(i >= 0 && i < numCells && j >= 0 && j < numCells)
		  nearBins.push_back(partBins[i][j]);

	    //Find forces for each particle in the current bin 
	    std::vector<particle_t*> currBin = partBins[r][c];
	    for(int p = 0; p < currBin.size(); ++p)
	    {
	       currBin[p]->ax = currBin[p]->ay = 0;

	       //Iterate through the nearby bins (nbi -> near bin index)
	       for(int nbi = 0; nbi < nearBins.size(); ++nbi)
	       {
		  //Get the current neighbor and iterate through particles and apply force
		  std::vector<particle_t*> neighbor = nearBins[nbi];

		  //Iteration through nearby particles (nbp)
		  for(int nbp = 0; nbp < neighbor.size(); ++nbp)
			  apply_force(*currBin[p], *neighbor[nbp], &dmin, &davg, &navg);
	       }
	    }
	  }
	}
 
        //
        //  move particles
        //

#pragma omp for
        for( int i = 0; i < n; i++ ) 
            move( particles[i] );		

        if( find_option( argc, argv, "-no" ) == -1 )
        {
          //
          // Computing statistical data
          //

#pragma omp master
          if (navg) {
            absavg +=  davg/navg;
            nabsavg++;
          }

#pragma omp critical
          if (dmin < absmin) absmin = dmin;
		
          //
          //  save if necessary
          //
#pragma omp master
          if( fsave && (step%SAVEFREQ) == 0 )
              save( fsave, n, particles );
        }
    }

    //End parallel section
    }
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, threads = %d, simulation time = %g seconds", n, numThreads, simulation_time);

    if( find_option( argc, argv, "-no" ) == -1 )
    {
      if (nabsavg) absavg /= nabsavg;
    // 
    //  -the minimum distance absmin between 2 particles during the run of the simulation
    //  -A Correct simulation will have particles stay at greater than 0.4 (of cutoff) with typical values between .7-.8
    //  -A simulation were particles don't interact correctly will be less than 0.4 (of cutoff) with typical values between .01-.05
    //
    //  -The average distance absavg is ~.95 when most particles are interacting correctly and ~.66 when no particles are interacting
    //
    printf( ", absmin = %lf, absavg = %lf", absmin, absavg);
    if (absmin < 0.4) printf ("\nThe minimum distance is below 0.4 meaning that some particle is not interacting");
    if (absavg < 0.8) printf ("\nThe average distance is below 0.8 meaning that most particles are not interacting");
    }
    printf("\n");     

    //
    // Printing summary data
    //
    if( fsum) 
        fprintf(fsum,"%d %g\n",n,simulation_time);
 
    //
    // Clearing space
    //
    if( fsum )
        fclose( fsum );    
    free( particles );
    if( fsave )
        fclose( fsave );
    
    return 0;
}
