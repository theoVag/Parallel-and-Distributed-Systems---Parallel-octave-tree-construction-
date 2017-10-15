#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "utils.h"
#define DIM 3
inline unsigned int compute_code(float x, float low, float step) {

  return floor((x - low) / step);

}

/* Function that does the quantization */
void quantize(unsigned int * codes, float * X, float * low, float step, int N) {

  __cilkrts_end_cilk(); // ensure that cilk is ended before setting number of workers
  if (__cilkrts_set_param("nworkers", num_threads) != 0) { //set number of workers
    //printf("Failed to set worker count\n");
  }
  __cilkrts_init();//initialize cilk
  //int numWorkers = __cilkrts_get_nworkers();
  //printf("We changed the number of workers in hash to %d.\n",numWorkers);
  for (int j = 0; j < DIM; j++) {//changing values scan to use cilk for
    cilk_for(int i = 0; i < N; i++) {//parallel for
      codes[i * DIM + j] = compute_code(X[i * DIM + j], low[j], step);

    }

  }
  __cilkrts_end_cilk();//end cilk
}

float max_range(float * x) {

  float max = -FLT_MAX;
  for (int i = 0; i < DIM; i++) {
    if (max < x[i]) {
      max = x[i];
    }
  }

  return max;

}

void compute_hash_codes(unsigned int * codes, float * X, int N,
  int nbins, float * min,
  float * max) {

  float range[DIM];
  float qstep;

  for (int i = 0; i < DIM; i++) {
    range[i] = fabs(max[i] - min[i]); // The range of the data
    range[i] += 0.01 * range[i]; // Add somthing small to avoid having points exactly at the boundaries 
  }

  qstep = max_range(range) / nbins; // The quantization step 

  quantize(codes, X, min, qstep, N); // Function that does the quantization

}
