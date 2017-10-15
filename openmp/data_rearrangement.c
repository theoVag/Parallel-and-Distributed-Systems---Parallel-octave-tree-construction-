#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "utils.h"
#include "omp.h"

#define DIM 3


void data_rearrangement(float *Y, float *X, 
			unsigned int *permutation_vector, 
			int N){
int i;//initialize i to use it in parallel for
#pragma omp parallel for private(i) num_threads(num_th)  //parallel for with variable i private for each thread, and the number of threads=num_th
	for(i=0; i<N; i++){

		memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
	}
}
