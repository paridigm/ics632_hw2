//
// gcc exercise1.c -o exercise1 -fopenmp -D PARALLEL_J
//
// perf stat -d ./exercise1
//
//

#ifndef N
	
	// #define DEBUG
	// #define N 790  	  // default array size
	
	#define N 200	
	
#endif

#include <omp.h>
#include <stdio.h>

#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

// globally defined arrays
int A[N][N];
int B[N][N];
int C[N][N];

void timer_handler (int signum) {
    printf ("Timed out!\n");
}

int main(){
	
	// add timeout
	struct itimerval timer;
	struct sigaction sa;
	
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (SIGVTALRM, &sa, 0);
	
	timer.it_value.tv_sec = 2;
	timer.it_value.tv_usec = 500000;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer (ITIMER_VIRTUAL, &timer, 0);
	
	
	
	// init num threads
	omp_set_num_threads(4);
	printf("Num threads: %d\n", omp_get_num_threads());
	
	// init vars
	int checksum = 0;
	int i, j, k;
	
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
	
		// causing bad values. added atomic.
	
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
		
		// looked at results, huge amounts of L1 cache misses
		// probably a huge number of calls to RAM memory is extremely slow
	
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
	
} 

