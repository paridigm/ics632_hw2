//
// gcc exercise1.c -o exercise1 -fopenmp -D PARALLEL_J
//
//

#ifndef N
	
	// #define DEBUG
	// #define N 790  	  // default array size
	
	#define N 200	
	
#endif

#include <omp.h>
#include <stdio.h>

// globally defined arrays
int A[N][N];
int B[N][N];
int C[N][N];

#ifdef DEBUG
int D[N][N];  // debug sequential
#endif

int main(){
	
	omp_set_num_threads(10);
	printf("Num threads: %d\n", omp_get_num_threads());
	
	int checksum = 0;
	int i, j, k;
	
	#ifdef DEBUG
	// initialize debug values
	int debugsum = 0;
	for (i=0; i < N; i++) { for (j=0; j < N; j++) {D[i][j] = 0;}}
	#endif
	
	// initialize values
	for (i=0; i < N; i++) {
		for (j=0; j < N; j++) {
			A[i][j] = i + j;
			B[i][j] = i + j;
			C[i][j] = 0;
		}
	}
	
	
	#ifdef PARALLEL_I
	
		// works fine
	
		printf("parallel i\n");
	
		#pragma omp parallel for private(i, k, j)
		for (i=0; i < N; i++) {
			for (k=0; k < N; k++) {
				for (j=0; j < N; j++) {
					C[i][j] += A[i][k] * B[k][j];
				}
			}
		}
	
	#elif PARALLEL_K
	
		// causing bad values.
	
		printf("parallel k\n");
	
		for (i=0; i < N; i++) {
			
			#pragma omp parallel for private(k, j)
			for (k=0; k < N; k++) {
				for (j=0; j < N; j++) {
					#pragma omp atomic
					C[i][j] += A[i][k] * B[k][j];
				}
			}
			
		}
	
	#elif PARALLEL_J
	
		// causing longer time than sequential. no idea why.
		
		// perhaps terrible caching?
		
		// tons of calls to RAM
	
		printf("parallel j\n");
	
		for (i=0; i < N; i++) {
			for (k=0; k < N; k++) {
				
				#pragma omp parallel for private(j)
				for (j=0; j < N; j++) {
					C[i][j] += A[i][k] * B[k][j];
				}
				
			}
		}
	
	#else
	
		// sequential matrix multiplication
		for (i=0; i < N; i++) {
			for (k=0; k < N; k++) {
				for (j=0; j < N; j++) {
					C[i][j] += A[i][k] * B[k][j];
				}
			}
		}
	
	#endif
	
	// calculate diagnoal checksum
	for (i=0; i < N; i++) { checksum += C[i][i]; }
	printf("checksum: %d\n", checksum);
	
	
	#ifdef DEBUG
	// debug ~ sequential math
	for (i=0; i < N; i++) {
		for (k=0; k < N; k++) {
			for (j=0; j < N; j++) {
				D[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	for (i=0; i < N; i++) { debugsum += D[i][i]; }
	printf("debugsum: %d\n", debugsum);
	#endif
	
} 




