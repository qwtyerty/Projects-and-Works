#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <cuda.h>
#include "common.h"

#define NUM_THREADS 256
#define get_bin_idx(p, num_bins, s)  (int)(p.x / (double)(s/num_bins)) + (int)(p.y / (double)(s / num_bins)) * num_bins
extern double size;


/**
  GPU Parallel Implementation of particule calculations
    Simulates the interaction of n particles with a correctness check
    Each thread handles a set of particles which calculates a set of bins to calculate only
      the necessary particles who are close enough to a target particle to have any bearing on the 
      forces between each other.
**/


//
//  benchmarking program
//

class bin_t
{
  public:
    int counter; // Counter for indexing current particle
    int next_counter; // Counter for indexing particles for the next step
    int prev_counter; // Counter for indexing particles for the previous step
    int particles[]; // Indexes for particles
    int part_next[16]; // Indexes of particles for the next step
    int part_prev[16]; // Indexes of particles for the previous step

    bin_t()
    {
	  this->next_counter = 0;
	  this->prev_counter = 0;
	  this->counter = 0;
    }

    //Adds a particle id to the end
    __host__ __device__ void append(int p_id)
    {
	  this->particles[this->counter] = p_id;
	  this->counter++;
    }

    //Increments the proper counter and appends to the proper array
    __host__ __device__ void update(int new_bin, int cur_bin, int p_id)
    {
	  if(cur_bin != new_bin)
	  {
		  ++this->prev_counter;
		  this->part_prev[this->prev_counter] = p_id;
	  }
	  else
	  {
		  ++this->next_counter;
		  this->part_next[this->next_counter] = p_id;
	  }
    }

    //Resets both the next counter and the prev counter
    __host__ __device__ void reset_counters()
    {
	    this->prev_counter = this->next_counter = 0;
    }

    //Swaps next to current to start the next step
    __host__ __device__ void next(int p_id)
    {
	    this->particles[p_id] = this->part_next[p_id];
    }
};

__device__ void apply_force_gpu(particle_t &particle, particle_t &neighbor)
{
  double dx = neighbor.x - particle.x;
  double dy = neighbor.y - particle.y;
  double r2 = dx * dx + dy * dy;
  if( r2 > cutoff*cutoff )
      return;
  //r2 = fmax( r2, min_r*min_r );
  r2 = (r2 > min_r*min_r) ? r2 : min_r*min_r;
  double r = sqrt( r2 );

  //
  //  very simple short-range repulsive force
  //
  double coef = ( 1 - cutoff / r ) / r2 / mass;
  particle.ax += coef * dx;
  particle.ay += coef * dy;

}

__global__ void compute_forces_gpu(particle_t * particles, bin_t* bins, int num_bins, int row_size ,int n)
{
  // Get thread (particle) ID
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  if(tid >= num_bins) return;

  int row = tid % row_size;
  int col = tid / row_size;

  for(int p = 0; p < bins[tid].counter; ++p)
  {
	  particles[bins[tid].particles[p]].ax = particles[bins[tid].particles[p]].ay = 0;
  }

  for(int r = row - 1; r <= row + 1; ++r)
  {
	  for(int c = col - 1; c <= col + 1; ++c)
	  {
		  //Bounds checking
		  if( r >= 0 && r < row_size && c >= 0 && c < row_size)
		  {

			  int nb_bin = c + r * row_size;
			  //Apply forces for each particle pair curr_part and nb_part
			  for(int curr_part = 0; curr_part < bins[tid].counter; ++curr_part)
			  {
				  for(int nb_part = 0; nb_part < bins[nb_bin].counter; ++nb_part)
				  {
					  apply_force_gpu(particles[bins[tid].particles[curr_part]],particles[bins[nb_bin].particles[nb_part]]);
				  }
			  }
		  }
	  }
  }
}



__device__ void move_gpu (particle_t  &p, double size)
{
    //
    //  slightly simplified Velocity Verlet integration
    //  conserves energy better than explicit Euler method
    //
    p.vx += p.ax * dt;
    p.vy += p.ay * dt;
    p.x  += p.vx * dt;
    p.y  += p.vy * dt;

    //
    //  bounce from walls
    //
    while( p.x < 0 || p.x > size )
    {
        p.x  = p.x < 0 ? -(p.x) : 2*size-p.x;
        p.vx = -(p.vx);
    }
    while( p.y < 0 || p.y > size )
    {
        p.y  = p.y < 0 ? -(p.y) : 2*size-p.y;
        p.vy = -(p.vy);
    }

}

__global__ void move_bins_gpu(particle_t* particles, bin_t* bins, int num_bins, int row_size, double size)
{
	int tid = threadIdx.x + blockIdx.x * blockDim.x;
	if(tid >= num_bins) return;

	bins->reset_counters();
	for(int p = 0; p < bins[tid].counter; ++p)
	{
		int next_p = bins[tid].particles[p];
		particle_t &part = particles[next_p];
		move_gpu(part, size);
		int new_bin_idx = get_bin_idx(part, row_size, size);
		bins[tid].update(new_bin_idx, tid, next_p);
	}
}

__global__ void bin_gpu(particle_t* particles, bin_t* bins, int num_bins, int row_size, double size)
{
	int tid = threadIdx.x + blockIdx.x * blockDim.x;
	if(tid >= num_bins) return;

	bins[tid].counter = bins[tid].next_counter;
	for(int p = 0; p < bins[tid].counter; ++p)
		bins[tid].next(p);
	int row = tid % row_size;
	int col = tid / row_size;
	for(int r = row - 1; r <= row + 1; ++r)
	{
		for(int c = col - 1; c <= col + 1; ++c)
		{
			if(r >= 0 && r < row_size && c >= 0 && c < row_size)
			{
				int target_bin = c + r * row_size;
				for(int p = 0; p < bins[target_bin].prev_counter; ++p)
				{
					int inc_part = bins[target_bin].part_prev[p];
					particle_t &part = particles[inc_part];
					if(get_bin_idx(part, row_size, size) == tid)
					{
						bins[tid].append(inc_part);
					}
				}
			}
		}
	}

}

int main( int argc, char **argv )
{    
    // This takes a few seconds to initialize the runtime
    cudaThreadSynchronize(); 

    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set the number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }
    
    int n = read_int( argc, argv, "-n", 1000 );

    char *savename = read_string( argc, argv, "-o", NULL );
    
    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;
    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );

    // GPU particle data structure
    particle_t * d_particles;
    cudaMalloc((void **) &d_particles, n * sizeof(particle_t));

    set_size( n );

    init_particles( n, particles );

    cudaThreadSynchronize();
    int row_size = size / 0.02;
    int num_bins = row_size * row_size;

    bin_t* bins = new bin_t[num_bins];
    for(int i = 0; i < n; ++i)
    {
	    int bin_idx = get_bin_idx(particles[i], row_size, size);
	    bins[bin_idx].append(i);
    } 

    bin_t* bins_gpu;
    cudaMalloc((void **) &bins_gpu, num_bins * sizeof(bin_t));
    cudaMemcpy(bins_gpu, bins, num_bins * sizeof(bin_t), cudaMemcpyHostToDevice);

    cudaThreadSynchronize();
    double copy_time = read_timer( );

    // Copy the particles to the GPU
    cudaMemcpy(d_particles, particles, n * sizeof(particle_t), cudaMemcpyHostToDevice);

    cudaThreadSynchronize();
    copy_time = read_timer( ) - copy_time;
    
    //
    //  simulate a number of time steps
    //
    cudaThreadSynchronize();
    double simulation_time = read_timer( );

    for( int step = 0; step < NSTEPS; step++ )
    {
        //
        //  compute forces
        //

	int blks = (n + NUM_THREADS - 1) / NUM_THREADS;
	cudaThreadSynchronize();
	compute_forces_gpu <<< blks, NUM_THREADS >>> (d_particles, bins_gpu, num_bins, row_size, n);
        //
        //  move particles
        //
	move_bins_gpu <<< blks, NUM_THREADS >>> (d_particles, bins_gpu, num_bins, row_size, size);
        //
	// Rebin for the next step
	//
	bin_gpu<<<blks, NUM_THREADS>>>(d_particles, bins_gpu, num_bins, row_size, size);

        //
        //  save if necessary
        //
        if( fsave && (step%SAVEFREQ) == 0 ) {
	    // Copy the particles back to the CPU
            cudaMemcpy(particles, d_particles, n * sizeof(particle_t), cudaMemcpyDeviceToHost);
            save( fsave, n, particles);
	}
    }
    cudaThreadSynchronize();
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "CPU-GPU copy time = %g seconds\n", copy_time);
    printf( "n = %d, simulation time = %g seconds\n", n, simulation_time );
    
    free( particles );
    cudaFree(d_particles);
    cudaFree(bins_gpu);
    if( fsave )
        fclose( fsave );
    
    return 0;
}
