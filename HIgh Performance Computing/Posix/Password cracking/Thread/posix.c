#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>
#include <pthread.h>

/******************************************************************************
 
  Compile with:
    cc -o CrackAZ99-With-Data CrackAZ99-With-Data.c -lcrypt

  

    ./CrackAZ99-With-Data > results.txt

  
******************************************************************************/
int n_pword = 4;
char *encrypt_pword[] = {
  "$6$KB$yPvAP5BWyF7oqOITVloN9mCAVdel.65miUZrEel72LcJy2KQDuYE6xccHS2ycoxqXDzW.lvbtDU5HuZ733K0X0",
  "$6$KB$H8RIsbCyr2r7L1lklCPKY0tLK9k5WudNWpxkNbx2bCBRHCsI3qyVRY.0nrovdkDLDJRsogQE9mA3OqcIafVsV0",
  "$6$KB$zd0hywe3NHS5T209L69g1LWlJjEXoT7OCwzWs.tL5mjK6.DlCY8azuxxy/ucDmebOMWKgWvS/A2..Ht1MyfI1/",
  "$6$KB$yRaZFVbx3SBqrI7KCFC572n/v5hWxyQXR3Y8WVbh05yX6GNeBGWN1tTCPT51Etagi4EQ8Vdd91ldNm6tIrIV40"
};
/**
 Required by lack of standard function in C.   
*/
void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}
/**
 This function can crack the kind of password explained above. All combinations
 that are tried are displayed and when the password is found, #, is put at the 
 start of the line. Note that one of the most time consuming operations that 
 it performs is the output of intermediate results, so performance experiments 
 for this kind of program should not include this. i.e. comment out the printfs.
*/
void kernel_1(char *salt_and_encrypted){
  int r, i, m;     // Loop counters
  char salt[7];    // String used in hashing the password. Need space for \0
  char plain[7];   // The combination of letters currently being checked
  char *enc;       // Pointer to the encrypted password
  int count = 0;   // The number of combinations explored so far

  substr(salt, salt_and_encrypted, 0, 6);

  for(r='A'; r<='O'; r++){
    for(i='A'; i<='Z'; i++){
      for(m=0; m<=99; m++){
        sprintf(plain, "%c%c%02d", r, i, m); 
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

void kernel_2(char *salt_and_encrypted){
  int r, i, m;     // Loop counters
  char salt[7];    // String used in hashing the password. Need space for \0
  char plain[7];   // The combination of letters currently being checked
  char *enc;       // Pointer to the encrypted password
  int count = 0;   // The number of combinations explored so far

  substr(salt, salt_and_encrypted, 0, 6);

  for(r='P'; r<='Z'; r++){
    for(i='A'; i<='Z'; i++){
      for(m=0; m<=99; m++){
        sprintf(plain, "%c%c%02d", r, i, m); 
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
pthread_t th_1; 
void kernel_1();
pthread_t th_2;
void kernel_2();

for(g=0;g<n_pword;g<g++) {

pthread_create(&th_1, NULL, (void *)kernel_1, encrypt_pword[g]);

pthread_create(&th_2, NULL, (void *)kernel_2, encrypt_pword[g]);

}


pthread_join(th_1, NULL);
pthread_join(th_2, NULL);

clock_gettime(CLOCK_MONOTONIC, &finish);
timedifference(&start, &finish, &timeelapsed);
printf("The time taken is: %lldns or %0.9lfs\n", timeelapsed,  (timeelapsed/1.0e9)); 

  return 0;
}
