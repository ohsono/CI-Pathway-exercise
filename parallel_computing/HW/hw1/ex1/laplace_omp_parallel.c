/*************************************************
 * Laplace OpenMP C Version
 *
 * Temperature is initially 0.0
 * Boundaries are as follows:
 *
 *      0         T         0
 *   0  +-------------------+  0
 *      |                   |
 *      |                   |
 *      |                   |
 *   T  |                   |  T
 *      |                   |
 *      |                   |
 *      |                   |
 *   0  +-------------------+ 100
 *      0         T        100
 *
 *  John Urbanic, PSC 2014
 *
 ************************************************/

/*************************************************
 * Optimized Laplace OpenMP C Version - Red-Black Algorithm
 * Key optimizations:
 * - Single grid (50% memory reduction)
 * - No expensive copying/swapping
 * - Better cache locality
 * - SIMD-friendly inner loops
 * - Optimal for modern CPUs
*************************************************/


#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

// size of plate
#define COLUMNS    1000
#define ROWS       1000

// largest permitted change in temp (This value takes about 3400 steps)
#define MAX_TEMP_ERROR 0.01

double Temperature[ROWS+2][COLUMNS+2] __attribute__((aligned(64))); //dynamic memory allocation with 64-byte
//double Temperature_last[ROWS+2][COLUMNS+2]; // temperature grid from last iteration

//   helper routines
void initialize();
void track_progress(int iter);

#define MAX_THREADS 32

int main(int argc, char *argv[]) {

    int i, j;                                            // grid indexes
    int max_iterations;                                  // number of iterations
    int iteration=1;                                     // current iteration
    double dt=100;                                       // largest change in t
    struct timeval start_time, stop_time, elapsed_time;  // timers

    // Set number of threads at runtime
    int num_threads = MAX_THREADS;
    if (omp_get_max_threads() < MAX_THREADS) {
        num_threads = omp_get_max_threads();
    }
    omp_set_num_threads(num_threads);
    printf("Running with %d OpenMP threads\n", num_threads);

    printf("Maximum iterations [100-4000]?\n");
    scanf("%d", &max_iterations);

    gettimeofday(&start_time,NULL); // Unix timer
    initialize();                   // initialize Temp_last including boundary conditions

    // do until error is minimal or until max steps
    while ( dt > MAX_TEMP_ERROR && iteration <= max_iterations ) {

        dt=0.0; // reset largest temperature change

        // Process RED squares (checkerboard pattern)
        double red_dt=0.0;
        // main calculation: average my four neighbors
        #pragma omp parallel for reduction(max:red_dt) private(i,j) schedule(static)
        for(i = 1; i <= ROWS; i++) {
            // SIMD
            #pragma omp simd aligned(Temperature:64) reduction(max:red_dt)
            for(j = 1 + (i % 2); j <= COLUMNS; j += 2) {
                double old_temp = Temperature[i][j];
                
                Temperature[i][j] = 0.25 * (Temperature[i+1][j] + Temperature[i-1][j] +
                                            Temperature[i][j+1] + Temperature[i][j-1]);
                red_dt = fmax(fabs(Temperature[i][j] - old_temp), red_dt);
            }
        }
        
        // BLACK squares
        double black_dt = 0.0;
        // copy grid to old grid for next iteration and find latest dt
        #pragma omp parallel for reduction(max:black_dt) private(i,j) schedule(static)
        for(i = 1; i <= ROWS; i++){
            #pragma omp simd aligned(Temperature:64) reduction(max:black_dt)
            for(j = 1 + ((i + 1) % 2); j <= COLUMNS; j += 2){
                double old_temp = Temperature[i][j];
                Temperature[i][j] = 0.25 * (Temperature[i+1][j] + Temperature[i-1][j] +
                                           Temperature[i][j+1] + Temperature[i][j-1]);
                black_dt = fmax( fabs(Temperature[i][j]-old_temp), black_dt);

            }
        }
        dt = fmax(red_dt, black_dt);


        // periodically print test values
        if((iteration % 100) == 0) {
 	        track_progress(iteration);
        }

	    iteration++;
    }

    gettimeofday(&stop_time,NULL);
    timersub(&stop_time, &start_time, &elapsed_time); // Unix time subtract routine

    printf("\nMax error at iteration %d was %f\n", iteration-1, dt);
    printf("Total time was %f seconds.\n", elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);

    return 0;
}

// initialize plate and boundary conditions
// Temp_last is used to to start first iteration
void initialize(){

    int i,j;

    #pragma omp parallel for private(i,j)
    for(i = 0; i <= ROWS+1; i++){
        for (j = 0; j <= COLUMNS+1; j++){
            Temperature[i][j] = 0.0;
        }
    }

    // these boundary conditions never change throughout run
    #pragma omp parallel
    {
        // set left side to 0 and right to a linear increase
        #pragma omp for 
        for(i = 0; i <= ROWS+1; i++) {
            Temperature[i][0] = 0.0;
            Temperature[i][COLUMNS+1] = (100.0/ROWS)*i;
        }
        // set top to 0 and bottom to linear increase
        #pragma omp for    
        for(j = 0; j <= COLUMNS+1; j++) {
            Temperature[0][j] = 0.0;
            Temperature[ROWS+1][j] = (100.0/COLUMNS)*j;
        }
    }
}

// print diagonal in bottom right corner where most action is
void track_progress(int iteration) {

    int i;

    printf("---------- Iteration number: %d ------------\n", iteration);
    for(i = ROWS-5; i <= ROWS; i++) {
        printf("[%d,%d]: %5.2f  ", i, i, Temperature[i][i]);
    }
    printf("\n");
}