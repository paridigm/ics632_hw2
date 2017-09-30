//
// gcc exercise2_fast.c -o exercise2_fast -fopenmp
//
// perf stat ./exercise2_fast threads iters
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <omp.h>

#define N 12000				// array size

#ifndef TILE_SIZE
	#define TILE_SIZE 300 	// tile size
#endif

#define Y j
#define X i-j
#define X2 t-j+i

int A[N+2][N+2];		// heat array
int t;					// index of last tile

int main(int argc, char **argv) {

  int num_threads;
  int num_iterations;

  int iter;
  int i,j,v,u;

  
  if (argc != 3) {
    fprintf(stderr,"Usage: %s <num threads> <num iterations>\n", argv[0]);
    exit(1);
  }

  if (sscanf(argv[1],"%d",&num_threads) != 1) {
    fprintf(stderr,"Invalid number of threads\n");
    exit(1);
  }

  if (sscanf(argv[2],"%d",&num_iterations) != 1) {
    fprintf(stderr,"Invalid number of iterations\n");
    exit(1);
  }

  // init num threads
  omp_set_num_threads(num_threads);
  //printf("Num threads: %d\n", num_threads);
  
  // get index of last tile
  t = (N%TILE_SIZE == 0)? N - TILE_SIZE + 1 : (N/TILE_SIZE)*TILE_SIZE + 1;
  //printf("LAST TILE INDEX: %d\n", t);
  
  // Fill in the array
  for (i=0; i < N+2; i++) {
    for (j=0; j < N+2; j++) {
      A[i][j] = i+j;
    }
  }

  // Loop for num_iterations iterations
  for (iter = 0; iter < num_iterations; iter++) {
	
	// for each diagnoal
	for(i=1+1; i<=t+1; i+=TILE_SIZE) {
		
		#pragma omp parallel shared(A, i) private(j, v, u) 
		{
			
			// decrement j ~ this can be parallelized to any order of j
			#pragma omp for 
			for(j=i-1; j>=1; j-=TILE_SIZE) {
				
				//int y = j; int x = i-j;
				//printf("%d %d\n", y, x);
				
				// perform checks for overflow
				for (v = Y; v < Y+TILE_SIZE && v < N+1; v++) {
					for (u = X; u < X+TILE_SIZE && u < N+1; u++) {
						
						A[v][u] = (3*A[v-1][u] + A[v+1][u] + 3*A[v][u-1] + A[v][u+1])/4;
						//printf(" %d %d, ", v, u);
						
					}
					//printf("\n");
				}
				
				
			}
			
		}
		
		//printf("\nSYNC\n\n");
		
	}
	
	//printf("\nNEXT PHASE\n\n");
	
	// for each diagnoal
	for(i=TILE_SIZE+1; i<=t; i+=TILE_SIZE) {
		
		#pragma omp parallel shared(A, i, t) private(j, v, u) 
		{
			
			// decrement j ~ this can be parallelized to any order of j
			#pragma omp for 
			for(j=t; j>=i; j-=TILE_SIZE) {
				
				//int y = j; int x = t-j+i;
				//printf("%d %d\n", y, x);
				
				// perform checks for overflow
				for (v = Y; v < Y+TILE_SIZE && v < N+1; v++) {
					for (u = X2; u < X2+TILE_SIZE && u < N+1; u++) {
						
						A[v][u] = (3*A[v-1][u] + A[v+1][u] + 3*A[v][u-1] + A[v][u+1])/4;
						//printf(" %d %d, ", v, u);
						
					}
					//printf("\n");
				}
				
			}
			
		}
		
		//printf("\nSYNC\n\n");
		
	}
	
	
  // end iterations
  }

	
  #ifdef CHECKSUM
	  // Compute and print the sum of elements for correctness checking
	  int sum =0 ;
	  for (i=1; i < N; i++) {
		for (j=1; j < N; j++) {
		  sum += A[i][j];
		}
	  }
	  fprintf(stderr,"sum = %d\n",sum);
  #endif

  exit(0);
}
