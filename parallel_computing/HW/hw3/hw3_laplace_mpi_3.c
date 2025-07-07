/****************************************************************
 * 2 Dimension Optimized Laplace MPI C Version                                         
 *                                                               
 * Performance Optimizations:
 * - Non-blocking communication with computation overlap
 * - Pointer swapping instead of array copying
 * - Loop fusion for better cache locality
 * - AllReduce instead of Reduce+Bcast
 * - Dynamic memory allocation for scalability
 * - Removed hardcoded processor count limitation
 *                                                               
 * T is initially 0.0                                            
 * Boundaries are as follows                                     
 *                                                               
 *                T                      4 sub-grids            
 *   0  +-------------------+  0    +-------------------+       
 *      |                   |       |                   |           
 *      |                   |       |-------------------|         
 *      |                   |       |                   |      
 *   T  |                   |  T    |-------------------|             
 *      |                   |       |                   |     
 *      |                   |       |-------------------|            
 *      |                   |       |                   |   
 *   0  +-------------------+ 100   +-------------------+         
 *      0         T       100                                    
 *                                                                 
 * Each PE only has a local subgrid.
 * Each PE works on a sub grid and then sends         
 * its boundaries to neighbors.
 *
 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define COLUMNS      1000
#define ROWS_GLOBAL  1000        // this is a "global" row count

// communication tags
#define DOWN     100
#define UP       101   

#define MAX_TEMP_ERROR 0.01

// Global pointers for dynamic arrays
double (*Temperature)[COLUMNS+2];
double (*Temperature_last)[COLUMNS+2];

void initialize(int npes, int my_PE_num, int my_rows);
void track_progress(int iteration, int my_rows);

int main(int argc, char *argv[]) {

    int i, j;
    int max_iterations;
    int iteration=1;
    double dt;
    struct timeval start_time, stop_time, elapsed_time;

    int        npes;                // number of PEs
    int        my_PE_num;           // my PE number
    double     dt_global=100;       // delta t across all PEs
    MPI_Status status;              // status returned by MPI calls
    MPI_Request requests[4];        // for non-blocking communication
    int        req_count;           // number of active requests
    
    // Dynamic row distribution
    int rows_per_process;
    int extra_rows;
    int my_rows;
    int my_start_row;

    // the usual MPI startup routines
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_PE_num);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);

    // Calculate dynamic load balancing
    rows_per_process = ROWS_GLOBAL / npes;
    extra_rows = ROWS_GLOBAL % npes;
    
    // Distribute extra rows to first few processes
    if (my_PE_num < extra_rows) {
        my_rows = rows_per_process + 1;
        my_start_row = my_PE_num * my_rows;
    } else {
        my_rows = rows_per_process;
        my_start_row = extra_rows * (rows_per_process + 1) + 
                       (my_PE_num - extra_rows) * rows_per_process;
    }

    // Allocate dynamic memory
    Temperature = (double (*)[COLUMNS+2])malloc((my_rows+2) * (COLUMNS+2) * sizeof(double));
    Temperature_last = (double (*)[COLUMNS+2])malloc((my_rows+2) * (COLUMNS+2) * sizeof(double));
    
    if (!Temperature || !Temperature_last) {
        printf("PE %d: Memory allocation failed\n", my_PE_num);
        MPI_Finalize();
        exit(1);
    }

    // PE 0 asks for input
    if(my_PE_num==0) {
        printf("Maximum iterations [100-4000]?\n");
        printf("Running on %d processes with dynamic load balancing\n", npes);
        fflush(stdout);
        scanf("%d", &max_iterations);
    }

    // bcast max iterations to other PEs
    MPI_Bcast(&max_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_PE_num==0) gettimeofday(&start_time,NULL);

    initialize(npes, my_PE_num, my_rows);

    while ( dt_global > MAX_TEMP_ERROR && iteration <= max_iterations ) {

        // PHASE 1: Start non-blocking communication for ghost rows
        req_count = 0;
        
        // Send bottom real row down and receive top ghost row
        if(my_PE_num != npes-1) {
            MPI_Isend(&Temperature_last[my_rows][1], COLUMNS, MPI_DOUBLE, 
                     my_PE_num+1, DOWN, MPI_COMM_WORLD, &requests[req_count++]);
        }
        if(my_PE_num != 0) {
            MPI_Irecv(&Temperature[0][1], COLUMNS, MPI_DOUBLE, 
                     my_PE_num-1, DOWN, MPI_COMM_WORLD, &requests[req_count++]);
        }
        
        // Send top real row up and receive bottom ghost row
        if(my_PE_num != 0) {
            MPI_Isend(&Temperature_last[1][1], COLUMNS, MPI_DOUBLE, 
                     my_PE_num-1, UP, MPI_COMM_WORLD, &requests[req_count++]);
        }
        if(my_PE_num != npes-1) {
            MPI_Irecv(&Temperature[my_rows+1][1], COLUMNS, MPI_DOUBLE, 
                     my_PE_num+1, UP, MPI_COMM_WORLD, &requests[req_count++]);
        }

        // PHASE 2: Calculate interior points (can overlap with communication)
        // Interior points don't need ghost cells
        for(i = 2; i < my_rows; i++) {
            for(j = 1; j <= COLUMNS; j++) {
                Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
                                           Temperature_last[i][j+1] + Temperature_last[i][j-1]);
            }
        }

        // PHASE 3: Wait for communication completion
        if (req_count > 0) {
            MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        }

        // PHASE 4: Calculate boundary rows that need ghost cells
        // Top boundary row (row 1)
        for(j = 1; j <= COLUMNS; j++) {
            Temperature[1][j] = 0.25 * (Temperature_last[2][j] + Temperature[0][j] +
                                       Temperature_last[1][j+1] + Temperature_last[1][j-1]);
        }
        
        // Bottom boundary row (row my_rows)
        for(j = 1; j <= COLUMNS; j++) {
            Temperature[my_rows][j] = 0.25 * (Temperature[my_rows+1][j] + Temperature_last[my_rows-1][j] +
                                             Temperature_last[my_rows][j+1] + Temperature_last[my_rows][j-1]);
        }

        // PHASE 5: Calculate convergence with loop fusion and pointer swapping
        dt = 0.0;
        for(i = 1; i <= my_rows; i++){
            for(j = 1; j <= COLUMNS; j++){
                dt = fmax(fabs(Temperature[i][j] - Temperature_last[i][j]), dt);
            }
        }

        // Pointer swapping instead of array copying
        double (*temp_ptr)[COLUMNS+2] = Temperature_last;
        Temperature_last = Temperature;
        Temperature = temp_ptr;

        // find global dt using AllReduce (more efficient than Reduce+Bcast)
        MPI_Allreduce(&dt, &dt_global, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        // periodically print test values - only for PE in lower corner
        if((iteration % 100) == 0) {
            if (my_PE_num == npes-1){
                track_progress(iteration, my_rows);
            }
        }

        iteration++;
    }

    // Slightly more accurate timing and cleaner output 
    MPI_Barrier(MPI_COMM_WORLD);

    // PE 0 finish timing and output values
    if (my_PE_num==0){
        gettimeofday(&stop_time,NULL);
        timersub(&stop_time, &start_time, &elapsed_time);

        printf("\nMax error at iteration %d was %f\n", iteration-1, dt_global);
        printf("Total time was %f seconds.\n", elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);
        printf("Grid size: %dx%d, Processes: %d\n", ROWS_GLOBAL, COLUMNS, npes);
    }

    // Clean up dynamic memory
    free(Temperature);
    free(Temperature_last);

    MPI_Finalize();
    return 0;
}

void initialize(int npes, int my_PE_num, int my_rows){

    double tMin, tMax;  //Local boundary limits
    int i,j;

    // Initialize all cells to 0.0
    for(i = 0; i <= my_rows+1; i++){
        for (j = 0; j <= COLUMNS+1; j++){
            Temperature_last[i][j] = 0.0;
        }
    }

    // Calculate this PE's portion of the global boundary
    int rows_per_process = ROWS_GLOBAL / npes;
    int extra_rows = ROWS_GLOBAL % npes;
    int my_start_row;
    
    if (my_PE_num < extra_rows) {
        my_start_row = my_PE_num * (rows_per_process + 1);
    } else {
        my_start_row = extra_rows * (rows_per_process + 1) + 
                       (my_PE_num - extra_rows) * rows_per_process;
    }

    // Local boundary condition endpoints
    tMin = my_start_row * 100.0 / ROWS_GLOBAL;
    tMax = (my_start_row + my_rows) * 100.0 / ROWS_GLOBAL;

    // Left and right boundaries
    for (i = 0; i <= my_rows+1; i++) {
        Temperature_last[i][0] = 0.0;
        Temperature_last[i][COLUMNS+1] = tMin + ((tMax-tMin)/my_rows)*i;
    }

    // Top boundary (PE 0 only)
    if (my_PE_num == 0)
        for (j = 0; j <= COLUMNS+1; j++)
            Temperature_last[0][j] = 0.0;

    // Bottom boundary (Last PE only)
    if (my_PE_num == npes-1)
        for (j=0; j<=COLUMNS+1; j++)
            Temperature_last[my_rows+1][j] = (100.0/COLUMNS) * j;
}

// only called by last PE
void track_progress(int iteration, int my_rows) {

    int i;

    printf("---------- Iteration number: %d ------------\n", iteration);

    // output global coordinates so user doesn't have to understand decomposition
    for(i = 5; i >= 0; i--) {
        printf("[%d,%d]: %5.2f  ", ROWS_GLOBAL-i, COLUMNS-i, Temperature_last[my_rows-i][COLUMNS-i]);
    }
    printf("\n");
}