#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "utils.h"
#include "pthread.h"
#define DIM 3
struct morton_args{//structure for passing the arguments into threads function
	unsigned long int *mcodes;
	unsigned int *codes;
	int N,max_level,start;
	
};
inline unsigned long int splitBy3(unsigned int a){
    unsigned long int x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}

inline unsigned long int mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z){
    unsigned long int answer;
    answer = splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
    return answer;
}
//threads_function
void *pthread_morton(void*a){
	struct morton_args *args=(struct morton_args*)a;//type casting void args
	 for(int i=0; i<args->N; i++){
    // Compute the morton codes from the hash codes using the magicbits mathod
    args->mcodes[args->start+i] = mortonEncode_magicbits(args->codes[(args->start)*DIM+i*DIM], args->codes[(args->start)*DIM+i*DIM + 1], args->codes[(args->start)*DIM+i*DIM + 2]);//adding start to point to the right data for each thread
    }
	pthread_exit(NULL);
}
/* The function that transform the morton codes into hash codes */ 
void morton_encoding(unsigned long int *mcodes, unsigned int *codes, int N, int max_level){
	int i=0,start,temp,rc;
	//allocate space for arguments' array and threads' array
	pthread_t *threads;
	struct morton_args *args;
	threads=(pthread_t *)malloc(num_threads*sizeof(pthread_t));
	args=(struct morton_args*)malloc(num_threads*sizeof(struct morton_args));
	
	for(i=0;i<num_threads;i++){
		//Distribute work to threads
		start=i*(N/num_threads);//point to the right start of jobs for each thread
		temp=N/num_threads;//point to the right amount of jobs for each thread
		//putting arguments into arguments' array
		args[i].codes = codes;
		args[i].mcodes = mcodes;
		args[i].max_level = max_level;
		args[i].N = temp;
		args[i].start=start;
        rc = pthread_create(&threads[i], NULL, pthread_morton, (void *)&args[i]);//creating array of threads

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


