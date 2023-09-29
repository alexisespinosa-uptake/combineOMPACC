#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

// grid size
#define GRIDY    8192
#define GRIDX    8192

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

// smallest permitted change in temperature
#define MAX_TEMP_ERROR 0.02

// Factor in the GRID for data to report in prints
#define FACDATA 0.98

double T_new[GRIDX+2][GRIDY+2]; // temperature grid
double T[GRIDX+2][GRIDY+2];     // temperature grid from last iteration

//   initialisation routine
void init();

int main(int argc, char *argv[]) {

    int i, j;                                            // grid indexes
    int max_iterations;                                  // maximal number of iterations
    int iteration=1;                                     // iteration
    double dt=100;                                       // largest change in temperature
    struct timeval start_time, stop_time, elapsed_time;  // timers

    if(argc!=2) {
      printf("Usage: %s number_of_iterations\n",argv[0]);
      exit(1);
    } else {
      max_iterations=atoi(argv[1]);
    }

    gettimeofday(&start_time,NULL); 

    init();                  

    #pragma omp target data map(tofrom:T) map(alloc:T_new)
    // simulation iterations
    while ( dt > MAX_TEMP_ERROR && iteration <= max_iterations ) {

        // reset dt
        dt = 0.0;

        // main computational kernel, average over neighbours in the grid
        #pragma omp target
        #pragma omp teams distribute
        for(i = 1; i <= GRIDX; i++) 
	    #pragma omp parallel for
            for(j = 1; j <= GRIDY; j++) 
                T_new[i][j] = 0.25 * (T[i+1][j] + T[i-1][j] +
                                            T[i][j+1] + T[i][j-1]);

        // compute the largest change and copy T_new to T
        #pragma omp target map(dt)
        #pragma omp teams distribute reduction(max:dt)
        for(i = 1; i <= GRIDX; i++){
            #pragma omp parallel for reduction(max:dt)
            for(j = 1; j <= GRIDY; j++){
	            dt = MAX( fabs(T_new[i][j]-T[i][j]), dt);
	            T[i][j] = T_new[i][j];
            }
        }

        // periodically print largest change
        if((iteration % 100) == 0) 
            printf("Iteration %4.0d, dt %f, T[Fac*GX][Fac*GY]=%f\n",iteration,dt,
                    T[(int)(FACDATA*(float)GRIDX)][(int)(FACDATA*(float)GRIDY)]);        
	     iteration++;
    }

    //------ Do we have T in the host ready to be saved?
    printf("Final values, iteration %4.0d, dt %f, T[Fac*GX][Fac*GY]=%f\n",iteration,dt,
              T[(int)(FACDATA*(float)GRIDX)][(int)(FACDATA*(float)GRIDY)]);

    gettimeofday(&stop_time,NULL);
    timersub(&stop_time, &start_time, &elapsed_time); // measure time

    printf("Total time was %f seconds.\n", elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);

    return 0;
}


// initialize grid and boundary conditions
void init(){

    int i,j;

    for(i = 0; i <= GRIDX+1; i++){
        for (j = 0; j <= GRIDY+1; j++){
            T[i][j] = 0.0;
        }
    }

    // these boundary conditions never change throughout run

    // set left side to 0 and right to a linear increase
    for(i = 0; i <= GRIDX+1; i++) {
        T[i][0] = 0.0;
        T[i][GRIDY+1] = (128.0/GRIDX)*i;
    }
    
    // set top to 0 and bottom to linear increase
    for(j = 0; j <= GRIDY+1; j++) {
        T[0][j] = 0.0;
        T[GRIDX+1][j] = (128.0/GRIDY)*j;
    }
}
