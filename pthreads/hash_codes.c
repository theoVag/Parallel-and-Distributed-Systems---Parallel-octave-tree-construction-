#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include "pthread.h"
#include "utils.h"
#define DIM 3

struct quantize_args{//structure for passing the arguments into threads' function
	unsigned int *codes;
	float *X,*low;
	float step;
	int N;
	int start;	
	
};
inline unsigned int compute_code(float x, float low, float step){

  return floor((x - low) / step);

}


/* Function that does the quantization */
//threads' function
void *pthread_quantize(void *a){
	struct quantize_args *args=(struct quantize_args *)a;//type casting void args
	
	for(int i=0; i<args->N; i++){
    for(int j=0; j<DIM; j++){
      args->codes[args->start+i*DIM + j] = compute_code(args->X[args->start+i*DIM + j], args->low[j], args->step); //adding start variable to point to the right data for each thread
    }
  }
	
	pthread_exit(NULL);
}

void quantize(unsigned int *codes, float *X, float *low, float step, int N){

	int i=0,start,temp,rc;
	//allocate space for arguments' array and threads' array
	struct quantize_args *args;
	pthread_t *threads;
	threads=(pthread_t *)malloc(num_threads*sizeof(pthread_t));
	args=(struct quantize_args *)malloc((num_threads*sizeof(struct quantize_args)));

	for(i = 0; i < num_threads; i++)
    {
		//distribute work to threads
		start=i*(N/num_threads)*DIM;//point to the right start of jobs for each thread
		temp=N/num_threads;//point to the right amount of jobs for each thread
		//putting arguments into arguments' array
		args[i].codes = codes;
		args[i].X = X;
		args[i].low = low;
		args[i].step = step;
		args[i].N = temp;
		args[i].start=start;
        rc = pthread_create(&threads[i], NULL, pthread_quantize, (void *)&args[i]);//creating array of threads

        if(rc)
        {
            printf("Error in pthread_create\n");
            return;
        }
    }
	
	for(i=0;i<num_threads;i++){//waiting all threads to end by joining them
		rc=pthread_join(threads[i],NULL);
		if(rc){
			printf("Error in pthread_join");
			return;
		}
	}
}

float max_range(float *x){

  float max = -FLT_MAX;
  for(int i=0; i<DIM; i++){
    if(max<x[i]){
      max = x[i];
    }
  }

  return max;

}

void compute_hash_codes(unsigned int *codes, float *X, int N, 
			int nbins, float *min, 
			float *max){
  
  float range[DIM];
  float qstep;

  for(int i=0; i<DIM; i++){
    range[i] = fabs(max[i] - min[i]); // The range of the data
    range[i] += 0.01*range[i]; // Add somthing small to avoid having points exactly at the boundaries 
  }

  qstep = max_range(range) / nbins; // The quantization step 
  
  quantize(codes, X, min, qstep, N); // Function that does the quantization

}



