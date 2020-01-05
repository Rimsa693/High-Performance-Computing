#include <stdio.h>
#include <cuda_runtime_api.h>
#include <time.h>
/****************************************************************************
  This program gives an example of a poor way to implement a password cracker
  in CUDA C. It is poor because it acheives this with just one thread, which
  is obviously not good given the scale of parallelism available to CUDA
  programs.
  
  The intentions of this program are:
    1) Demonstrate the use of __device__ and __global__ functions
    2) Enable a simulation of password cracking in the absence of library 
       with equivalent functionality to libcrypt. The password to be found
       is hardcoded into a function called is_a_match.   

  Compile and run with:
    nvcc -o passwordcrack_cuda passwordcrack_cuda.cu
    ./passwordcrack_cuda
   
  Dr Kevan Buckley, University of Wolverhampton, 2018
*****************************************************************************/

/****************************************************************************
  This function returns 1 if the attempt at cracking the password is 
  identical to the plain text password string stored in the program. 
  Otherwise,it returns 0.
*****************************************************************************/

__device__ int is_a_match(char *attempt) {
	char passr1[] = "RA1234";
	char passr2[] = "AN3029";
	char passr3[] = "GO1998";
	char passr4[] = "SA0023";


	char *a = attempt;
	char *b = attempt;
	char *c = attempt;
	char *d = attempt;
	char *d1 = passr1;
	char *d2 = passr2;
	char *d3 = passr3;
	char *d4 = passr4;

	while(*a == *d1) { 
		if(*a == '\0') 
		{
			printf("Password: %s\n",passr1);
			break;
		}

		a++;
		d1++;
	}
	
	while(*b == *d2) { 
		if(*b == '\0') 
		{
			printf("Password: %s\n",passr2);
			break;
		}

		b++;
		d2++;
	}

	while(*c == *d3) { 
		if(*c == '\0') 
		{
			printf("Password: %s\n",passr3);
			break;
		}

		c++;
		d3++;
	}

	while(*d == *d4) { 
		if(*d == '\0') 
		{
			printf("Password: %s\n",passr4);
			return 1;
		}

		d++;
		d4++;
	}
	return 0;

}

__global__ void  kernel() {
	char a1,a2,a3,a4;

	char passr[7];
	passr[6] = '\0';

	int i = blockIdx.x+65;
	int j = threadIdx.x+65;
	char firstMatch = i; 
	char secondMatch = j; 

	passr[0] = firstMatch;
	passr[1] = secondMatch;
	for(a1='0'; a1<='9'; a1++){
		for(a2='0'; a2<='9'; a2++){
			for(a3='0'; a3<='9'; a3++){
				for(a4='0'; a4<='9'; a4++){
					passr[2] = a1;
					passr[3] = a2;
					passr[4] = a3;
					passr[5] = a4;
					 
					if(is_a_match(passr)) {
					} 
					else {
	     			//printf("tried: %s\n", passr);		  
					}
				}
			}
		}
	}
}

int time_difference(struct timespec *start, 
	struct timespec *finish, 
	long long int *difference) {
	long long int dsecond =  finish->tv_sec - start->tv_sec; 
	long long int dnsecond =  finish->tv_nsec - start->tv_nsec; 
	if(dnsecond < 0 ) {
		dsecond--;
		dnsecond += 1000000000; 
	} 
	*difference = dsecond * 1000000000 + dnsecond;
	return !(*difference > 0);
}


int main() {

	struct  timespec start, finish;
	long long int time_elapsed;
	clock_gettime(CLOCK_MONOTONIC, &start);

	kernel <<<26,26>>>();
	cudaThreadSynchronize();

	clock_gettime(CLOCK_MONOTONIC, &finish);
	time_difference(&start, &finish, &time_elapsed);
	printf("Time elapsed was %lldnsecond or %0.9lfsecond\n", time_elapsed, (time_elapsed/1.0e9)); 

	return 0;
}


