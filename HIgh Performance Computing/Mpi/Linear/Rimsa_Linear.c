#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>


/******************************************************************************
 * To compile:
 *   mpicc -o Rimsa_Linear Rimsa_Linear.c -lm

 * To run:
 *   mpirun -n 9 ./Rimsa_Linear
 *****************************************************************************/

typedef struct point_t {
  double x;
  double y;
} point_t;

int n_data = 1000;
point_t data[];

double residual_error(double x, double y, double m, double c) {
  double e = (m * x) + c - y;
  return e * e;
}

double rms_error(double m, double c) {
  int i;
  double mean;
  double error_sum = 0;
  
  for(i=0; i<n_data; i++) {
    error_sum += residual_error(data[i].x, data[i].y, m, c);  
  }
  
  mean = error_sum / n_data;
  
  return sqrt(mean);
}

int timedifference(struct timespec *start, struct timespec *finish, long long int *difference) {
   long long int dsec = finish->tv_sec - start->tv_sec;
   long long int dnsec = finish->tv_nsec - start->tv_nsec;

	if(dnsec < 0) {
		dsec--;
		dnsec += 1000000000; 
	}

   *difference = dsec * 1000000000 + dnsec;
   return !(*difference > 0);
}

int main() {

  struct timespec start, finish;
  long long int timeelapsed;
  clock_gettime(CLOCK_MONOTONIC, &start);

  int rank, size;
  int i;
  double bm = 1.3;
  double bc = 10;
  double be;
  double dm[8];
  double dc[8];
  double e[8];
  double step = 0.01;
  double best_error = 999999999;
  int best_error_i;
  int minimum_found = 0;
  double pError = 0;
  double baseMC[2];
  
  double om[] = {0,1,1, 1, 0,-1,-1,-1};
  double oc[] = {1,1,0,-1,-1,-1, 0, 1};

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  be = rms_error(bm, bc);

  if(size != 9) {
    if(rank == 0) {
      printf("This program needs to run on exactly 9 processes\n");
      return 0;
     }
   } 

  while(!minimum_found)
  {
    if (rank != 0)
	{
		i = rank -1;
		dm[i] = bm + (om[i] * step);
		dc[i] = bc + (oc[i] * step);
		pError = rms_error (dm[i], dc[i]);

		MPI_Send (&pError, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send (&dm[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send (&dc[i], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

		MPI_Recv (&bm, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv (&bc, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv (&minimum_found, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
    else
	{
		for(i = 1; i < size; i++)
		{
			MPI_Recv (&pError, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv (&dm[i-1] , 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv (&dc[i-1], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			if(pError < best_error)
			{
				best_error = pError;
				best_error_i = i - 1;
			}
		}

		if(best_error < be)
		{
			be = best_error;
			bm = dm[best_error_i];
			bc = dc[best_error_i];
		}
		
		else
		{
			minimum_found = 1;
		}
		
		for(i = 1; i < size; i++)
		{
			MPI_Send (&bm, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			MPI_Send (&bc, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			MPI_Send (&minimum_found, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

		}
	}
    }

    if(rank == 0)
      {
	printf ("minimum m,c is %lf, %lf with error %lf\n", bm, bc, be);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	timedifference(&start, &finish, &timeelapsed);
	printf("TIme elasped: %lldnsec or %0.9lfsec\n", timeelapsed, (timeelapsed/1.0e9));
      }

  MPI_Finalize();
  return 0;
}
point_t data[] =  {
  {77.71,118.37},{79.99,117.25},{69.06,122.31},{69.44,97.80},
  {78.39,133.72},{75.10,116.40},{72.92,100.75},{66.00,107.78},
  {78.42,135.07},{ 1.98,27.38},{44.30,98.30},{12.89,54.55},
  {50.28,81.23},{ 0.87,32.41},{60.11,103.89},{61.07,95.05},
  {57.58,94.71},{13.53,42.89},{85.53,147.68},{66.85,96.47},
  {89.65,145.23},{59.58,114.64},{53.38,85.22},{10.20,40.31},
  {83.60,114.19},{ 7.48,17.31},{80.00,117.22},{52.39,86.20},
  {53.09,98.96},{90.75,128.71},{97.80,146.45},{19.76,30.07},
  {73.05,98.86},{47.12,96.59},{16.89, 7.56},{42.79,65.89},
  {78.08,116.52},{22.14,35.28},{ 8.32,34.35},{80.45,118.18},
  {13.46,30.08},{98.94,148.92},{14.74,61.82},{23.09,60.07},
  {73.49,134.11},{38.21,66.26},{ 7.08,13.11},{40.65,102.52},
  { 4.92,26.25},{72.74,128.55},{25.25,33.78},{71.87,115.97},
  {73.70,125.19},{75.21,121.60},{47.52,85.10},{51.48,77.13},
  {69.78,112.68},{74.89,120.82},{41.59,76.25},{ 6.25,21.02},
  {53.77,92.85},{83.60,133.92},{13.30,33.67},{81.35,146.79},
  {20.63,47.13},{75.03,113.14},{29.91,61.69},{40.32,64.97},
  {26.41,64.73},{30.93,48.34},{50.20,66.71},{83.38,135.77},
  {46.28,84.61},{52.76,98.56},{89.02,133.43},{47.94,69.37},
  {86.83,127.83},{41.09,72.44},{86.41,124.49},{75.35,112.22},
  {27.19,45.42},{79.23,122.70},{79.94,122.33},{ 2.55,29.54},
  {23.62,34.45},{17.62,60.87},{61.60,110.86},{33.60,43.98},
  {98.29,129.36},{96.42,159.26},{97.06,153.63},{23.54,63.41},
  {92.21,124.73},{93.80,143.77},{99.21,141.99},{37.77,76.65},
  {60.85,108.71},{32.82,76.23},{58.21,99.66},{41.05,67.89},
  {20.40,44.63},{96.85,118.68},{93.03,151.95},{96.12,143.23},
  { 2.38,26.53},{74.99,117.20},{41.23,75.18},{ 1.22,15.65},
  {86.09,140.03},{32.13,71.68},{ 5.24,36.52},{ 3.37,31.88},
  {88.79,143.02},{74.29,132.13},{78.39,133.92},{48.90,83.32},
  {35.85,61.90},{61.94,99.71},{55.87,100.07},{53.60,98.75},
  { 7.87,57.98},{18.03,54.82},{38.16,64.94},{97.60,152.45},
  {83.75,132.52},{ 7.46,35.02},{45.36,90.96},{14.30,52.57},
  {91.74,134.30},{84.93,131.68},{91.39,143.15},{22.66,50.78},
  {56.21,114.58},{ 2.93,31.36},{24.46,59.05},{80.63,109.87},
  {11.44,37.10},{63.49,111.92},{ 9.43,36.66},{61.90,106.73},
  {33.41,64.37},{28.01,62.66},{68.99,115.81},{31.88,77.88},
  {41.96,66.67},{56.29,93.49},{54.32,89.16},{21.69,43.79},
  {98.14,141.51},{86.27,143.26},{13.78,39.65},{55.31,79.25},
  {78.78,129.62},{75.89,114.13},{62.17,99.07},{ 2.85,20.60},
  {26.17,58.13},{73.04,110.64},{82.63,138.70},{81.45,134.00},
  {83.51,126.17},{65.61,89.46},{87.80,156.70},{37.41,84.56},
  {66.08,117.32},{40.79,54.80},{33.53,65.25},{41.04,72.05},
  {66.82,96.09},{64.98,99.59},{ 0.48,14.04},{ 3.79,27.94},
  {75.75,112.43},{13.40,26.16},{71.22,124.57},{76.30,127.05},
  {20.79,32.42},{50.82,96.31},{20.31,50.97},{90.14,139.39},
  {38.36,72.64},{30.21,58.75},{21.07,41.05},{49.10,85.66},
  {56.15,83.49},{95.58,145.48},{38.99,72.62},{77.33,127.70},
  {18.89,27.48},{60.60,92.75},{82.51,158.13},{36.16,78.41},
  {32.93,46.63},{95.76,156.67},{87.48,128.92},{39.63,67.11},
  {26.92,43.70},{21.68,43.05},{ 5.25,21.22},{94.31,151.40},
  {36.46,48.57},{86.11,143.05},{76.03,117.55},{93.51,148.78},
  {28.82,36.20},{42.91,63.78},{42.68,73.03},{ 1.56,23.66},
  {72.77,96.96},{12.89,37.54},{76.73,104.42},{13.11,37.49},
  {79.24,122.47},{19.77,51.97},{97.41,167.81},{36.15,66.21},
  {85.83,141.64},{97.81,140.42},{19.79,44.90},{60.73,100.93},
  {71.57,109.06},{61.08,99.11},{26.65,56.85},{83.86,118.62},
  {71.71,102.11},{95.39,157.38},{62.06,109.50},{51.76,96.57},
  {87.21,151.31},{42.84,87.45},{77.11,127.24},{93.93,132.40},
  {14.07,34.64},{76.22,107.11},{91.14,130.17},{41.13,93.39},
  {45.86,65.70},{44.29,84.05},{88.13,129.71},{83.78,128.84},
  {47.64,76.55},{43.71,89.37},{45.24,69.61},{41.40,73.34},
  {78.05,115.35},{73.60,130.53},{51.39,76.25},{ 5.36,37.32},
  {98.60,134.79},{55.74,107.52},{80.27,130.95},{55.86,112.57},
  {76.90,132.49},{70.12,99.17},{37.98,63.30},{59.69,87.79},
  {27.60,59.93},{ 7.85,13.81},{91.31,142.38},{61.71,90.80},
  { 3.53,20.92},{43.51,70.62},{67.48,111.76},{51.69,89.26},
  {42.72,73.83},{62.41,84.52},{ 6.93,39.03},{53.73,72.68},
  {78.51,134.77},{ 0.04,12.87},{32.23,69.84},{47.65,89.71},
  {20.63,40.80},{31.71,64.32},{79.65,119.23},{44.49,80.50},
  {15.85,71.07},{79.52,126.35},{49.54,76.21},{65.93,95.92},
  {80.63,109.78},{ 3.89,18.16},{78.08,132.04},{13.10,18.05},
  { 8.09,48.23},{71.70,102.45},{39.58,73.17},{50.35,87.03},
  { 1.63,19.98},{43.46,81.78},{20.67,30.42},{41.44,84.93},
  {48.79,82.65},{43.40,87.03},{27.04,78.79},{54.44,86.12},
  {25.95,59.95},{68.03,121.75},{31.42,61.04},{61.36,110.79},
  {21.85,64.55},{19.03,37.01},{67.99,130.99},{22.70,56.76},
  {13.20,28.61},{53.60,88.43},{ 9.53,37.45},{94.33,131.89},
  {85.92,136.44},{77.44,116.76},{85.34,119.62},{32.78,64.06},
  {33.77,74.14},{15.69,39.03},{45.25,68.73},{70.58,101.24},
  {81.07,121.24},{84.05,111.60},{28.02,49.85},{42.92,75.76},
  {64.33,114.30},{54.02,102.96},{25.63,34.36},{13.15,34.17},
  {72.20,102.12},{25.95,60.98},{11.33,32.68},{12.57,42.62},
  {75.81,118.49},{33.39,75.99},{47.08,78.23},{41.85,80.72},
  {32.80,54.82},{61.02,98.56},{51.27,83.86},{15.76,53.40},
  {16.74,39.72},{55.21,96.87},{41.06,87.73},{44.64,70.94},
  { 6.37,28.78},{72.32,102.31},{19.40,44.87},{33.49,43.03},
  {73.66,130.70},{33.05,63.86},{ 9.44,28.35},{93.85,143.55},
  {88.61,131.82},{18.18,32.11},{85.96,137.80},{62.64,108.88},
  {44.87,65.03},{ 7.97,29.72},{97.26,146.68},{88.75,128.80},
  {27.41,50.93},{29.15,57.13},{ 8.87,31.25},{ 4.10,41.36},
  {22.94,53.20},{55.52,107.71},{35.11,63.22},{28.63,60.14},
  {47.21,72.73},{36.78,67.44},{20.55,52.79},{76.51,136.88},
  {40.00,74.92},{69.58,118.32},{25.26,65.70},{24.34,55.95},
  {29.39,48.23},{55.09,98.80},{22.29,42.40},{ 3.57,33.11},
  {23.99,57.04},{25.25,61.68},{ 6.29, 4.79},{46.72,69.01},
  {88.43,154.62},{49.62,83.67},{57.38,108.12},{ 1.65,32.45},
  {14.21,40.08},{51.90,108.16},{23.59,62.75},{ 1.38,15.38},
  {72.16,110.86},{23.69,63.86},{26.81,68.93},{58.09,96.22},
  { 3.11,31.31},{16.93,23.06},{20.73,47.49},{68.43,112.12},
  {89.41,125.83},{38.03,70.19},{88.91,127.64},{15.60,37.27},
  {79.80,130.93},{58.62,94.38},{97.38,161.61},{82.35,133.83},
  {12.41,56.68},{76.26,130.66},{99.68,140.59},{41.95,69.47},
  {67.29,107.94},{63.46,119.31},{58.18,94.67},{52.51,70.40},
  { 4.92,22.25},{38.59,73.84},{93.82,142.33},{84.56,125.48},
  {27.33,73.83},{78.09,125.10},{27.32,52.72},{63.51,101.17},
  {69.23,107.72},{71.50,129.66},{47.09,88.65},{ 1.69,12.36},
  {14.43,29.88},{25.03,50.90},{87.32,138.41},{ 7.33,26.36},
  {42.44,73.18},{81.54,138.65},{21.00,42.17},{20.01,60.70},
  {10.91,50.60},{72.92,134.81},{25.72,47.36},{74.81,115.12},
  {43.02,69.35},{ 7.49,17.92},{16.01,62.76},{47.61,78.91},
  {63.03,114.84},{41.47,70.16},{10.99,43.14},{65.29,122.99},
  {84.13,151.79},{31.56,72.09},{42.02,66.99},{75.43,122.59},
  {66.67,108.12},{94.41,136.84},{65.70,104.84},{28.83,45.17},
  {83.23,115.45},{83.22,132.69},{25.34,40.08},{39.41,77.42},
  {86.43,137.37},{82.92,138.46},{77.39,130.12},{27.93,71.13},
  { 5.98,14.68},{53.22,102.60},{69.02,125.95},{31.21,52.17},
  {60.89,96.81},{72.29,127.61},{59.73,97.42},{ 3.41,34.91},
  {67.59,102.83},{ 3.52,25.26},{22.92,43.58},{ 9.56,35.27},
  {75.71,118.93},{74.50,99.32},{75.97,109.67},{98.54,144.18},
  {42.28,84.20},{11.03,49.30},{58.27,97.63},{68.86,115.18},
  {18.28,39.07},{94.18,140.02},{85.29,139.47},{90.94,122.07},
  {85.45,142.35},{24.99,57.95},{13.13,45.83},{11.05,36.79},
  {34.63,68.62},{82.21,123.38},{77.92,125.53},{49.74,101.96},
  {44.84,89.51},{55.42,82.02},{45.86,75.45},{75.20,123.93},
  {86.83,129.61},{55.84,96.01},{94.94,161.02},{ 6.08,40.37},
  {93.48,143.56},{69.31,108.07},{ 8.44,50.11},{90.19,124.44},
  { 7.76,39.72},{50.86,86.96},{75.60,120.34},{26.92,60.22},
  {90.61,147.35},{28.75,47.08},{10.09,29.92},{41.39,85.20},
  {42.61,89.96},{70.80,128.41},{95.80,150.46},{ 5.24,32.06},
  {38.48,81.51},{ 1.84,20.27},{76.81,115.18},{94.45,149.21},
  {97.84,147.80},{29.87,65.79},{89.72,124.70},{61.41,108.91},
  {61.92,93.34},{93.02,138.49},{40.00,99.11},{93.69,140.55},
  {49.15,79.54},{15.09,38.92},{72.51,110.53},{58.69,98.46},
  {19.89,43.37},{30.08,53.34},{65.85,108.89},{24.23,61.85},
  { 4.00,18.71},{83.31,136.71},{95.61,155.94},{ 6.06,27.71},
  {32.15,60.19},{52.35,87.88},{32.47,57.17},{18.16,31.09},
  {30.95,70.51},{ 3.06,28.73},{67.59,105.87},{32.85,72.58},
  {36.83,59.70},{94.38,143.76},{64.11,123.13},{ 3.82,47.87},
  { 0.05, 7.22},{97.38,138.16},{61.43,95.61},{48.82,71.95},
  {40.72,83.12},{27.25,49.60},{68.62,119.62},{38.86,86.99},
  {84.41,129.27},{50.41,94.42},{58.25,84.50},{76.15,115.62},
  {98.74,157.68},{85.86,123.04},{75.11,121.87},{ 2.08,24.96},
  { 0.61,16.67},{44.85,62.44},{24.40,56.84},{27.55,74.58},
  {35.04,50.21},{ 8.12,36.09},{82.93,122.09},{ 7.23,22.68},
  {84.75,149.08},{98.09,135.56},{44.72,82.11},{56.69,99.54},
  {73.44,108.31},{89.69,146.60},{15.68,52.36},{61.02,97.96},
  {82.44,125.94},{15.20,37.46},{95.25,133.23},{63.12,116.50},
  {61.00,90.26},{97.78,143.63},{26.50,63.40},{49.85,69.20},
  { 9.59,29.31},{65.87,108.15},{85.70,120.68},{24.60,49.25},
  {37.32,63.35},{24.52,39.37},{45.29,98.28},{ 2.40,23.86},
  {37.13,61.72},{18.83,46.26},{61.99,89.33},{10.78,35.44},
  {96.87,140.16},{74.81,124.22},{92.42,150.59},{ 3.93,27.67},
  {98.50,151.73},{83.89,138.01},{13.44,29.08},{12.43,63.45},
  {59.00,107.05},{ 8.87,39.62},{95.43,137.76},{33.46,78.39},
  {81.86,127.60},{62.80,82.03},{51.12,98.72},{ 6.46,22.45},
  {34.17,71.95},{46.53,62.89},{51.89,86.67},{99.81,159.73},
  {15.53,25.82},{27.02,53.90},{ 6.74,21.51},{ 8.39,46.49},
  { 4.18,26.44},{12.12,28.82},{12.32,29.01},{20.52,68.74},
  { 4.35,41.51},{36.92,40.93},{ 3.02,22.89},{31.95,65.75},
  {88.17,130.42},{47.20,73.61},{28.83,46.88},{83.22,129.91},
  { 1.91, 6.78},{67.76,92.99},{20.53,46.48},{11.65,37.44},
  { 6.69,19.11},{ 5.65,24.92},{46.45,67.67},{86.36,126.52},
  {53.65,92.55},{79.46,117.67},{ 1.93,18.91},{65.19,124.83},
  { 8.60,33.39},{53.01,88.02},{ 4.10,16.66},{19.55,47.08},
  {70.43,106.99},{68.46,131.30},{43.00,88.27},{ 1.09,23.86},
  {49.10,88.88},{38.80,71.23},{48.58,56.84},{17.51,49.43},
  {86.81,136.83},{32.99,80.49},{40.77,69.47},{ 8.96,43.94},
  {77.88,112.41},{90.41,130.55},{34.68,80.40},{26.12,38.12},
  {97.31,131.87},{83.20,128.30},{49.34,92.64},{74.72,109.87},
  {65.13,96.07},{40.33,95.63},{12.69,70.18},{93.04,123.67},
  {62.77,95.33},{10.01,42.56},{50.26,91.79},{33.03,64.88},
  {35.60,74.93},{22.34,71.49},{35.91,91.66},{63.35,107.85},
  {55.45,81.38},{75.28,114.90},{83.57,143.65},{27.74,55.13},
  {54.63,93.95},{77.31,140.26},{77.35,118.13},{77.60,134.28},
  {24.18,40.23},{93.52,148.38},{89.15,134.32},{50.77,87.31},
  {67.85,103.17},{78.51,139.13},{66.65,121.66},{55.62,100.25},
  {38.93,68.47},{35.20,54.12},{48.24,81.83},{ 2.03,17.58},
  {97.45,144.40},{47.17,72.08},{23.74,35.80},{25.37,48.72},
  { 5.31,15.37},{66.74,107.76},{48.65,97.69},{98.93,160.88},
  {69.73,115.68},{65.00,105.45},{36.58,86.29},{11.47,12.24},
  {35.17,59.65},{37.79,61.17},{27.99,66.00},{70.76,107.06},
  {36.48,43.24},{30.39,38.81},{74.79,97.82},{99.11,141.48},
  {65.18,94.20},{57.64,113.19},{60.38,105.02},{ 2.51,14.01},
  { 5.64,14.90},{99.02,152.02},{85.49,139.91},{51.13,88.82},
  {91.16,139.02},{59.68,110.37},{28.17,61.92},{64.77,118.17},
  {86.11,121.95},{ 0.65,29.67},{11.52,47.82},{55.48,89.30},
  {85.96,134.27},{17.73,60.43},{72.41,127.45},{98.91,132.37},
  {23.99,52.87},{75.61,122.24},{93.47,150.85},{38.10,89.12},
  {36.36,93.40},{14.67,35.45},{19.00,28.81},{34.13,56.63},
  {25.50,53.50},{66.38,106.13},{21.23,35.13},{58.52,101.30},
  {45.48,85.22},{18.94,45.86},{36.91,71.11},{68.31,102.83},
  {48.55,76.34},{83.76,119.65},{13.59,41.19},{25.11,59.64},
  {88.34,140.80},{40.73,65.81},{75.06,117.91},{34.52,70.34},
  {60.33,112.96},{93.45,159.26},{14.69,37.71},{67.94,108.60},
  {66.55,105.72},{29.61,67.84},{54.44,86.84},{85.79,124.78},
  {94.04,143.80},{47.76,103.96},{67.79,121.88},{ 4.08,26.63},
  {66.30,118.13},{58.84,109.51},{78.38,119.86},{71.80,125.94},
  {80.36,120.93},{43.39,55.98},{62.44,80.43},{59.86,100.00},
  {52.63,89.05},{87.47,133.26},{ 4.52,41.66},{67.69,95.44},
  {25.85,50.83},{62.84,125.73},{30.62,66.84},{41.36,72.66},
  {90.15,116.95},{47.89,80.34},{66.11,113.55},{62.44,117.34},
  { 7.46,18.13},{ 2.31,22.79},{ 3.48,-2.56},{69.38,105.80},
  {71.06,102.65},{21.22,43.33},{61.68,101.09},{82.58,128.02},
  {16.07,41.68},{67.61,100.83},{92.63,136.33},{46.09,77.04},
  {58.98,89.11},{47.14,62.19},{79.96,123.29},{69.28,107.76},
  {57.66,101.57},{ 8.75,22.96},{76.37,109.15},{16.16,49.22},
  {63.84,94.16},{50.39,80.51},{23.40,46.06},{ 1.19,11.59},
  {33.56,107.61},{21.15,43.11},{50.64,105.18},{49.37,82.33},
  { 3.04,20.08},{86.17,134.57},{93.84,164.91},{37.66,72.16},
  {37.60,76.78},{31.93,62.18},{24.76,64.81},{81.81,135.08},
  {19.47,44.59},{51.06,93.21},{75.95,105.78},{45.11,84.19},
  {26.09,53.01},{65.29,111.15},{16.30,43.46},{ 3.98,37.02},
  {31.01,83.67},{36.74,80.27},{57.44,98.20},{20.22,55.81},
  {90.30,142.05},{33.02,72.78},{10.00,30.94},{48.85,100.09},
  { 8.98,38.97},{16.55,50.11},{41.77,75.79},{85.21,117.09},
  { 7.76,16.77},{ 4.85,28.72},{51.86,86.07},{95.10,149.69},
  { 6.01,14.26},{88.88,149.08},{91.29,135.31},{90.04,127.70},
  {87.74,140.95},{82.55,134.69},{35.17,77.45},{24.19,51.84},
  {10.17,33.81},{91.68,139.46},{ 3.79,29.42},{78.61,127.21},
  {19.41,48.24},{56.46,83.05},{16.64,44.55},{67.28,114.50},
  {85.25,127.93},{90.71,131.57},{95.99,161.52},{ 2.47,22.18},
  {51.76,86.07},{16.01,47.34},{80.02,127.68},{76.54,104.42},
  {69.92,109.39},{15.84,43.24},{57.14,97.97},{10.51,36.98},
  {83.47,136.46},{75.61,106.65},{58.51,98.79},{69.56,124.26},
  {76.79,102.97},{26.32,62.18},{52.91,84.21},{68.31,100.40},
  {29.00,47.75},{47.07,80.94},{13.08,48.23},{21.29,47.84},
  {96.08,138.90},{73.92,111.23},{44.28,79.11},{38.58,71.59},
  {30.00,61.29},{59.70,102.57},{89.92,136.71},{22.75,50.13},
  {56.49,78.08},{85.09,123.94},{66.34,101.70},{81.95,139.69},
  {53.94,82.47},{53.59,109.38},{61.26,93.45},{85.31,153.15},
  {10.04,31.69},{46.56,82.32},{87.62,140.91},{37.29,68.26},
  {89.50,152.28},{64.42,118.71},{54.39,82.86},{40.36,65.47},
  {99.30,157.10},{58.89,110.67},{ 7.79,27.51},{30.56,57.76},
  {25.36,53.57},{30.97,52.82},{43.58,66.82},{42.36,65.37},
  {93.76,158.15},{82.47,125.24},{91.63,156.91},{78.72,105.92},
  {31.87,62.72},{82.22,127.72},{64.74,103.19},{31.55,59.89},
  {80.05,107.74},{92.46,143.29},{60.94,97.23},{68.53,108.77},
  {85.14,120.73},{68.38,122.31},{80.44,114.39},{39.21,62.39},
  {24.66,43.93},{22.87,49.86},{58.84,104.78},{99.98,153.75},
  {69.48,113.97},{34.39,57.97},{83.36,138.58},{71.68,114.77},
  {44.75,80.05},{ 9.39,40.75},{63.47,103.88},{47.26,84.64},
  {29.85,66.80},{16.55,63.13},{51.43,89.78},{69.16,95.26},
  {64.46,115.19},{63.15,104.10},{69.83,109.62},{99.17,146.82},
  {25.56,45.36},{51.14,84.62},{75.33,113.95},{29.84,80.08},
  {52.39,92.98},{79.23,113.80},{11.40,23.76},{26.31,58.95},
  {93.83,152.10},{53.62,74.22},{ 2.21,18.99},{16.19,58.50},
  { 0.69,26.72},{86.80,127.51},{39.37,82.93},{27.86,65.81},
  {64.34,90.19},{ 0.21,18.49},{16.40,42.73},{27.58,53.62},
  {31.50,75.06},{69.92,120.33},{93.68,145.77},{33.52,52.58},
  {44.61,66.12},{31.67,65.82},{50.45,70.54},{45.07,76.15}
};
