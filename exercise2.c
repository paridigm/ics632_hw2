//
//
// gcc exercise2_startingpoint.c -o exercise2_startingpoint -fopenmp -D PARALLEL
//
// perf stat ./exercise2_startingpoint 1 1
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <omp.h>

#define N 5

#define Y j
#define X i-j
#define X2 N-j+i

int A[N+2][N+2];

int main(int argc, char **argv) {

  int num_threads;
  int num_iterations;

  int iter;
  int i,j,k;

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

  // Fill in the array
  for (i=0; i < N+2; i++) {
    for (j=0; j < N+2; j++) {
      A[i][j] = i+j;
    }
  }

  // Loop for num_iterations iterations
  for (iter = 0; iter < num_iterations; iter++) {
	
	#ifdef PARALLEL
		
		// for each diagnoal
		for(i=2; i<=N+1; i++) {
			
			#pragma omp parallel shared(A, i) private(j) 
			{
				
				// decrement j ~ this can be parallelized to any order of j
				#pragma omp for 
				for(j=i-1; j>=1; j--) {
					
					A[Y][X] = (3*A[Y-1][X] + A[Y+1][X] + 3*A[Y][X-1] + A[Y][X+1])/4;
					
					//int y = j;
					//int x = i-j;
					//printf("%d %d\n", y, x);
					
				}
				
			}
			
			printf("\nSYNC\n\n");
			
		}
		
		
		
		// for each diagnoal
		for(i=2; i<=N; i++) {
			
			#pragma omp parallel shared(A, i) private(j) 
			{
				
				// decrement j ~ this can be parallelized to any order of j
				#pragma omp for 
				for(j=N; j>=i; j--) {
					
					A[Y][X2] = (3*A[Y-1][X2] + A[Y+1][X2] + 3*A[Y][X2-1] + A[Y][X2+1])/4;
					
					//int y = j;
					//int x = N-j+i;
					//printf("%d %d\n", y, x);
					
				}
				
			}
			
			printf("\nSYNC\n\n");
			
		}
		
		
		// TODO: for some reason, when I parallelize, my sum changes
		
		// The above code will be O(2N) with theoretically infinite parallel threads
	
	
	#elif ROW_COL 
	
		// go along diagnol
		for (i = 1; i < N+1; i++) {
		
			// initial value
			A[i][i] = (3*A[i-1][i] + A[i+1][i] + 3*A[i][i-1] + A[i][i+1])/4;

			// horizontals (thread)
			for (j = i+1; j < N+1; j++) {
				A[i][j] = (3*A[i-1][j] + A[i+1][j] + 3*A[i][j-1] + A[i][j+1])/4;
			}

			// verticals (other thread)
			for (k = i+1; k < N+1; k++) {
				A[k][i] = (3*A[k-1][i] + A[k+1][i] + 3*A[k][i-1] + A[k][i+1])/4;
			}
			
		}
	
		// The above code can only utilize 2 threads at once, therefore only twice as fast as the original
	
	
	#else
	
		// original
		for (i = 1; i < N+1; i++) {
			for (j = 1; j < N+1; j++) {
				A[i][j] = (3*A[i-1][j] + A[i+1][j] + 3*A[i][j-1] + A[i][j+1])/4;
			}
		}
		
		// O(N^2) 
		
	#endif
	
  }

  // Compute and print the sum of elements for correctness checking
  int sum =0 ;
  for (i=1; i < N; i++) {
    for (j=1; j < N; j++) {
      sum += A[i][j];
    }
  }
  
  // sum for correctness
  fprintf(stderr,"sum = %d\n",sum);

  exit(0);
}
