#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>

/******************************************************************************
 
  Compile with:
    cc -o CrackAZ99-With-Data CrackAZ99-With-Data.c -lcrypt

  

    ./CrackAZ99-With-Data > results.txt

  
******************************************************************************/
int n_pswd = 4;

char *encrypted_pswd[] = {
  "$6$KB$qtZDwvkMc3yVn8iBKfjD0LKtFdbszRAPZhxq53gL/6qPsyjogDmlewgujJhg9X76z25HWrYVN23aM9Yk9l6yg0",
  "$6$KB$r0eI.GvMdy.bF.P2hO7iNiyUSnr5RcYRrtlq8jJKtBWOJF8GUEL50wfl3tFezoHyeojVSvbI9lyyTPt8GrcQc1",
  "$6$KB$FAaDEzWHHZkDMIMeiTsMzuTeImWHvhk28RrAHP8HhcLuv9VgcgTuceVrBQ4vn1kwBYb75CBhYlD/aJ8f6py7L1",
  "$6$KB$yWpwGDOqIYzvXw/0G7lTPVdtqmRr8BxN6WzsbuzUOjI.OSf1KEP8VYgnw3619nn/M2BV8AMDvoPBgrVqaZoK91"
};





/**
 Required by lack of standard function in C.   
*/

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}



void crack(char *salt_and_encrypted){
  int r, i, m, s;     // Loop counters
  char salt[7];    // String used in hashing the password. Need space for \0
  char plain[7];   // The combination of letters currently being checked
  char *enc;       // Pointer to the encrypted password
  int count = 0;   // The number of combinations explored so far

  substr(salt, salt_and_encrypted, 0, 6);

  for(r='A'; r<='Z'; r++){
    for(i='A'; i<='Z'; i++){
	for(m='A'; m<='Z'; m++){    
	  for(s=0; s<=99; s++){
        sprintf(plain, "%c%c%c%02d", r, i, m, s); 
        enc = (char *) crypt(plain, salt);
        count++;
        if(strcmp(salt_and_encrypted, enc) == 0){
          printf("#%-8d%s %s\n", count, plain, enc);
        } else {
          printf(" %-8d%s %s\n", count, plain, enc);
        }
      }
    }
  }
  printf("%d Outcomes Count \n", count);
}

}

int timedifference(struct timespec *start, 
		   struct timespec *finish, 
long long int *difference) {
long long int dsecond = finish->tv_sec - start->tv_sec;
long long int dnanosecond = finish->tv_nsec - start->tv_nsec;

if(dnanosecond < 0) {
dsecond--;
dnanosecond += 1000000000; 
}
*difference = dsecond * 1000000000 + dnanosecond;
return !(*difference > 0);
}

int main(int argc, char *argv[]){
int g;
struct timespec start, finish;
long long int timeelapsed;


clock_gettime(CLOCK_MONOTONIC, &start);

 for(g=0;g<n_pswd;g<g++) {
    crack(encrypted_pswd[g]);
  }

clock_gettime(CLOCK_MONOTONIC, &finish);
timedifference(&start, &finish, &timeelapsed);
printf("The time taken is: %lldns or %0.9lfs\n", timeelapsed,  (timeelapsed/1.0e9)); 

return 0;
}
