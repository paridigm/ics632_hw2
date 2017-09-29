//
// gcc -fopenmp
//

#include <omp.h>
#include <stdio.h>

int main(){
	
	int i;
	
	omp_set_num_threads(10);
	
	printf("Start %d\n", omp_get_num_threads());
	
	#pragma omp parallel
	{
		printf("Hello World\n");
	}
	
	#pragma omp parallel private(i) 
	{
		
		#pragma omp for 
		for(i=0; i<10; i++) {
			printf("%d\n", i);
		}
		
	}
	
	/* Resume Serial Code */
	printf("Done\n");
	
} 




	/*
		// diag sum values for N+2 to N+N
		for(i=N+2; i<=N+N; i++) {
			
			//#pragma omp parallel shared(A) private(i, j) 
			//{
				
				// decrement j ~ can be parallelized to any order of j
				//#pragma omp for 
				for(j=N; j>=1 && (i-j) <= N; j--) {
					A[Y][X] = (3*A[Y-1][X] + A[Y+1][X] + 3*A[Y][X-1] + A[Y][X+1])/4;
				}
				
			//}
			//printf("\nSYNC\n\n");
		}
		*/