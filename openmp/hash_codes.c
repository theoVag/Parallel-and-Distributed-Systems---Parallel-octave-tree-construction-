#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "utils.h"
#include "omp.h"
#define DIM 3


inline unsigned int compute_code(float x, float low, float step)
{
  return floor((x - low) / step);
}


/* Function that does the quantization */
void quantize(unsigned int *codes, float *X,
    float *low, float step, int N)
{

  int i = 0, j = 0;
 
#pragma omp parallel for private(i, j) shared(X, low, step) num_threads(num_th) //parallel for with variable i private for each thread and shared variables for all threads, and the number of threads=num_th
    for (i = 0; i < N; ++i)
    {
      for (j = 0; j < DIM; ++j)
      {
        
        codes[i*DIM + j] = compute_code(X[i*DIM + j], low[j], step);
 	/*codes[i*DIM] = compute_code(X[i*DIM + 0], low[0], step);
	codes[i*DIM + 1] = compute_code(X[i*DIM + 1], low[1], step);
	codes[i*DIM + 2] = compute_code(X[i*DIM + 2], low[2], step); //remove loop carried dependence*/

      }
    }
  
}

inline float max_range(float *x)
{
  int i=0;
  float max = -FLT_MAX;
  for (i = 0; i < DIM; ++i)
  {
    if (max < x[i])
    {
      max = x[i];
    }
  }
  return max;
}

void compute_hash_codes(unsigned int *codes, float *X, int N,
			int nbins, float *min,
			float *max)
{
  float range[DIM];
  float qstep;
  int i = 0;

  for(i=0; i<DIM; i++)
  {
    range[i] = fabs(max[i] - min[i]);  // The range of the data
    // Add something small to avoid having points exactly at the boundaries
    range[i] += 0.01*range[i];
  }

  qstep = max_range(range) / nbins; // The quantization step

  quantize(codes, X, min, qstep, N); // Function that does the quantization
}


