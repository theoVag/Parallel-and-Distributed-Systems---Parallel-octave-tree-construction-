#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "pthread.h"
#include "utils.h"
#define MAXBINS 8
int counter;

pthread_mutex_t mut;//mutex variable

struct radix_args {//structure to pass the arguments into threads function
    unsigned long int *morton_codes;
    unsigned long int *sorted_morton_codes;
    unsigned int *permutation_vector;
    unsigned int *index;
    unsigned int *level_record;
    int N,population_threshold,sft,lv;
};


void *thread_handler(void*a);
inline void swap_long(unsigned long int **x, unsigned long int **y) {

    unsigned long int *tmp;
    tmp = x[0];
    x[0] = y[0];
    y[0] = tmp;

}

inline void swap(unsigned int **x, unsigned int **y) {

    unsigned int *tmp;
    tmp = x[0];
    x[0] = y[0];
    y[0] = tmp;

}

void truncated_radix_sort(unsigned long int *morton_codes,
                          unsigned long int *sorted_morton_codes,
                          unsigned int *permutation_vector,
                          unsigned int *index,
                          unsigned int *level_record,
                          int N,
                          int population_threshold,
                          int sft, int lv) {


    int BinSizes[MAXBINS] = {0};
    int BinCursor[MAXBINS] = {0};
    unsigned int *tmp_ptr;
    unsigned long int *tmp_code;
    int i,rc,temp;

    
    if(N<=0) {
		
        return;
    }
    else if(N<=population_threshold || sft < 0) { // Base case. The node is a leaf
		
        level_record[0] = lv; // record the level of the node
        memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
        memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes
		
        return;
    }
    else {
		if(lv==0){//initiallization of mutex,counter on level=0
			temp=0;
			pthread_mutex_init(&mut, NULL);
			counter=num_threads;
		}
        level_record[0] = lv;
        // Find which child each point belongs to
        for(int j=0; j<N; j++) {
            unsigned int ii = (morton_codes[j]>>sft) & 0x07;
            BinSizes[ii]++;
        }

        // scan prefix (must change this code)
        int offset = 0;
        for(int i=0; i<MAXBINS; i++) {
            int ss = BinSizes[i];
            BinCursor[i] = offset;
            offset += ss;
            BinSizes[i] = offset;
        }

        for(int j=0; j<N; j++) {
            unsigned int ii = (morton_codes[j]>>sft) & 0x07;
            permutation_vector[BinCursor[ii]] = index[j];
            sorted_morton_codes[BinCursor[ii]] = morton_codes[j];
            BinCursor[ii]++;
        }

        //swap the index pointers
        swap(&index, &permutation_vector);

        //swap the code pointers
        swap_long(&morton_codes, &sorted_morton_codes);


	if(counter>0){//if there are not available threads go serial skipping mutex_lock
			
        pthread_mutex_lock (&mut);//starting critical section
        //if available threads>8 then go parallel creating 8 threads
        if(counter>8) {//create 8 threads if counter>8
			//allocate space for arguments' array and threads' array
            struct radix_args *args;
            pthread_t *threads;
            threads=(pthread_t *)malloc(MAXBINS*sizeof(pthread_t));
            args=(struct radix_args *)malloc((MAXBINS*sizeof(struct radix_args)));

            for(int i=0; i<MAXBINS; i++) {

                int offset = (i>0) ? BinSizes[i-1] : 0;
                int size = BinSizes[i] - offset;

                counter--;//reduce counter when we create a thread
				// pass arguments to threads
                args[i].morton_codes=&morton_codes[offset];
                args[i].sorted_morton_codes=&sorted_morton_codes[offset];
                args[i].permutation_vector=&permutation_vector[offset];
                args[i].index=&index[offset];
                args[i].level_record=&level_record[offset];
                args[i].N=size;
                args[i].population_threshold=population_threshold;
                args[i].sft=sft-3;
                args[i].lv=lv+1;
				rc=pthread_create(&threads[i],NULL,thread_handler,(void*)&args[i]);//create thread
				if (rc){
					printf("Error in thread's creation \n");
					exit(-1);
				}              

            }
            pthread_mutex_unlock (&mut);// end of critical section
            for(i=0; i<MAXBINS; i++) {
                rc=pthread_join(threads[i],NULL);
                if(rc) {
                    printf("Error in pthread_join");
                    return;
                }
            }

        }
        else if(counter<=8 && counter!=0) {//if available threads<8 and bigger than 0 go parallel for available threads and continue serial for the rest of octal
         
            temp=counter;
            //allocate space for arguments' array and threads' array
            struct radix_args *args;
            pthread_t *threads;
            threads=(pthread_t *)malloc(temp*sizeof(pthread_t));
            args=(struct radix_args *)malloc((temp*sizeof(struct radix_args)));
            for(int i=0; i<temp; i++) {

                int offset = (i>0) ? BinSizes[i-1] : 0;
                int size = BinSizes[i] - offset;
                counter--;
                args[i].morton_codes=&morton_codes[offset];
                args[i].sorted_morton_codes=&sorted_morton_codes[offset];
                args[i].permutation_vector=&permutation_vector[offset];
                args[i].index=&index[offset];
                args[i].level_record=&level_record[offset];
                args[i].N=size;
                args[i].population_threshold=population_threshold;
                args[i].sft=sft-3;
                args[i].lv=lv+1;
             
				rc=pthread_create(&threads[i],NULL,thread_handler,(void*)&args[i]);
				if (rc){
					printf("Error in thread's creation \n");
					exit(-1);
				}
                
            }
            pthread_mutex_unlock (&mut);//end of critical section if the second if statement was true
            for(i=0; i<temp; i++) {
                rc=pthread_join(threads[i],NULL);
                if(rc) {
                    printf("Error in pthread_join");
                    return;
                }
            }
            
            
            for(int i=temp; i<MAXBINS; i++) {  //continue serial with the rest of MAXBINS
					int offset = (i>0) ? BinSizes[i-1] : 0;
					int size = BinSizes[i] - offset;
					
					truncated_radix_sort(&morton_codes[offset],
										 &sorted_morton_codes[offset],
										 &permutation_vector[offset],
										 &index[offset], &level_record[offset],
										 size,
										 population_threshold,
										 sft-3, lv+1);
				}
            

        }
        else if(counter==0) {//go serial for 1st time of serial execution and unlock mutex
            pthread_mutex_unlock (&mut); 
  
				for(int i=0; i<MAXBINS; i++) {
						int offset = (i>0) ? BinSizes[i-1] : 0;
						int size = BinSizes[i] - offset;
      
						truncated_radix_sort(&morton_codes[offset], 
							   &sorted_morton_codes[offset], 
							   &permutation_vector[offset], 
							   &index[offset], &level_record[offset], 
							   size, 
							   population_threshold,
							   sft-3, lv+1);
				}
				
			
        }
	}
	else {//go serial if there aren't available threads
		for(int i=0; i<MAXBINS; i++) {
				int offset = (i>0) ? BinSizes[i-1] : 0;
				int size = BinSizes[i] - offset;
      
				truncated_radix_sort(&morton_codes[offset], 
							   &sorted_morton_codes[offset], 
							   &permutation_vector[offset], 
							   &index[offset], &level_record[offset], 
							   size, 
							   population_threshold,
							   sft-3, lv+1);
		}
	}

  }
    
}
//threads' function
void *thread_handler(void*a) {
    struct radix_args *args=(struct radix_args*)a;
    unsigned long int *morton_codes=args->morton_codes;
    unsigned long int *sorted_morton_codes=args->sorted_morton_codes;
    unsigned int *permutation_vector=args->permutation_vector;
    unsigned int *index=args->index;
    unsigned int *level_record=args->level_record;
    int N=args->N;
    int population_threshold=args->population_threshold;
    int sft=args->sft;
    int lv=args->lv;
	
	truncated_radix_sort(morton_codes,sorted_morton_codes,permutation_vector,index,level_record,N,population_threshold,sft,lv);
    pthread_exit(NULL);
}


