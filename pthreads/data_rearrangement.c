#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "pthread.h"
#include "utils.h"

#define DIM 3
struct rear_args{ //structure for passing the arguments into threads' function
	float *Y;
	float *X;
	unsigned int *permutation_vector; 
	int N,start;

};
//threads' function
void *pthread_rear(void*a){
	struct rear_args *args=(struct rear_args*)a;//type casting void args
	
	for(int i=0; i<args->N; i++){
		
		memcpy(&(args->Y[i*DIM+args->start*DIM]), &(args->X[args->permutation_vector[args->start+i]*DIM]), DIM*sizeof(float));//adding start to point to the right data for each thread
    }
	pthread_exit(NULL);
}

void data_rearrangement(float *Y, float *X, 
			unsigned int *permutation_vector, 
			int N){

	int i=0,start,temp,rc;
	//allocate space for arguments array and threads array
	struct rear_args *args;
	pthread_t *threads;
	threads=(pthread_t *)malloc(num_threads*sizeof(pthread_t));
	args=(struct rear_args *)malloc((num_threads*sizeof(struct rear_args)));

	for(i=0;i<num_threads;i++){
		//distribute work to threads
		start=i*(N/num_threads);//point to the right start of jobs for each thread
		temp=N/num_threads;//point to the right amount of jobs for each thread
		//putting arguments into arguments' array
		args[i].X = X;
		args[i].Y = Y;
		args[i].permutation_vector=permutation_vector;
		args[i].N = temp;
		args[i].start=start;
		
        rc = pthread_create(&threads[i], NULL, pthread_rear, (void *)&args[i]);//creating array of threads

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
