#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "utils.h"
#define DIM 3


void data_rearrangement(float *Y, float *X, 
			unsigned int *permutation_vector, 
			int N){
	__cilkrts_end_cilk(); // ensure that cilk is ended before setting number of workers 
	if (__cilkrts_set_param("nworkers",num_threads)!=0){//set number of workers
		//printf("Failed to set worker count\n");
	}
	__cilkrts_init();//initialize cilk
	//int numWorkers = __cilkrts_get_nworkers();
	//printf("We changed the number of workers in rear to %d.\n",numWorkers);
  cilk_for(int i=0; i<N; i++){//parallel for with cilk
    memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
  }
	__cilkrts_end_cilk();//end cilk
}
