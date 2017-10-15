#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "utils.h"
#include "omp.h"
#define MAXBINS 8
int new_thread=0;
int ready,counter;
//counter=threads available for use
//ready=number of threads for creation
// new_thread=flag allows threads' creating
inline void swap_long(unsigned long int **x, unsigned long int **y){

  unsigned long int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}

inline void swap(unsigned int **x, unsigned int **y){

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
			  int sft, int lv){

  int BinSizes[MAXBINS] = {0};
  int BinCursor[MAXBINS] = {0};
  unsigned int *tmp_ptr;
  unsigned long int *tmp_code;


  if(N<=0){

    return;
  }
  else if(N<=population_threshold || sft < 0) { // Base case. The node is a leaf

    level_record[0] = lv; // record the level of the node
    memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
    memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes 

    return;
  }
  else{
	if(lv==0){//initialization for tha first time counter=num_th
		
		counter=num_th;
	}
    level_record[0] = lv;
    // Find which child each point belongs to 
    for(int j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      BinSizes[ii]++;
    }

    // scan prefix (must change this code)  
    int offset = 0;
    for(int i=0; i<MAXBINS; i++){
      int ss = BinSizes[i];
      BinCursor[i] = offset;
      offset += ss;
      BinSizes[i] = offset;
    }
    
    for(int j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      permutation_vector[BinCursor[ii]] = index[j];
      sorted_morton_codes[BinCursor[ii]] = morton_codes[j];
      BinCursor[ii]++;
    }
    
    //swap the index pointers  
    swap(&index, &permutation_vector);

    //swap the code pointers 
    swap_long(&morton_codes, &sorted_morton_codes);

    /* Call the function recursively to split the lower levels */
        
        #pragma omp critical //only one thread can change new_thread at a time
        {
			if (counter>0){
				new_thread = 1; //allow creating more threads
			}
			else if (counter=0 ){
				new_thread = 0; //don't allow creating more threads
			}
		}
	int i;	

        omp_set_nested(new_thread);// allow nested parallelism if new_thread==1
        #pragma omp critical //only one thread can change counter and variable ready each time
        {
			if(omp_get_nested()!=0 && counter>=MAXBINS){//if nested parallelism is enabled and available threads >=8 send 8 threads and reduce counter
				ready=MAXBINS;
				counter=counter-MAXBINS;
			}
			else if(omp_get_nested()!=0 && counter>0 && counter<MAXBINS){//if nested parallelism is enabled and available threads >0 send ready with number of threads and reduce counter
				ready=counter;
				counter=0;
				
			}
			else{// if available threads=0 send ready=1
				counter=0;
				ready=1;
			}
		}
        /* Call the function recursively to split the lower levels */
        #pragma omp parallel private(i) num_threads(ready)// parallel section private variable for each thread i and num_threads=ready
        {
            #pragma omp for nowait schedule(static) //nowait: threads don't wait jobs to complete, static: repetitions are equally splited and shared to each thread
    for(i=0; i<MAXBINS; i++){
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

