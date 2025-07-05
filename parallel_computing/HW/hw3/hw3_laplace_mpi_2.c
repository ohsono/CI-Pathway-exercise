/****************************************************************
 * Project: CI Pathway Summer 2025 
 * Course: Parallel Programing 
 * Assignment: Distributed Memory Parallelism                                                                 
 * Original Author: John Urbanic, PSC 2014
 * Author: Hochan Son, UCLA, Statistics
 * Date : 2025-06-30 
 
 * Note:
  - Implemented robust circular (ring) communication: Each PE exchanges boundary 
  rows with its neighbors using a deadlock-free pattern, supporting any number of 
  processes.
  - Improved boundary initialization: The initialze_circular function sets left/right 
  boundaries for all PEs, and top/bottom boundaries for the first/last PE, ensuring 
  proper boundary conditions.
  - Added periodic progress reporting: The code prints progress every 100 iterations
   from the last PE, showing representative grid values.
  - Added timing and result summary: The code measures and reports total runtime 
  and final error on PE 0.
  - Memory management: Dynamic allocation and cleanup of 2D arrays for temperature 
  grids.
  - Verbose control: Communication messages are printed only if the verbose flag is 
  enabled.
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
#define verbose 0

void initialze_circular(int ROWS, int npes, int my_PE_num, double **Temperature_last) {
    //All: generic boundary 
    for (int i = 0; i <= ROWS + 1; i++) {
        Temperature_last[i][0] = 0.0;           // Left boundary
        Temperature_last[i][COLUMNS+1] = 100.0; // Right boundary  
    }

    //PE_0: set top boundary
    if (my_PE_num == 0) {
        for (int j = 0; j <= COLUMNS + 1; j++)
            Temperature_last[0][j] = 0.0; // Top boundary
    }

    //PE_7: set bottom boundary
    if (my_PE_num == npes-1) {
        for (int j = 0; j <= COLUMNS + 1; j++)
            Temperature_last[ROWS+1][j] = 100.0; // Bottom boundary
    }
}

// only called by last PE
void track_progress(int iteration, int ROWS, double **Temperature) {

    printf("---------- Iteration number: %d ------------\n", iteration);
    // output global coordinates so user doesn't have to understand decomposition
    for(int i = 5; i >= 0; i--) {
      printf("[%d,%d]: %5.2f  ", ROWS-i, COLUMNS-i, Temperature[ROWS-i][COLUMNS-i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {
    int my_PE_num;                  // Current PE
    int npes;                       // number of PEs
    int next_PE, prev_PE;
    double dt, dt_global = 100;
    struct timeval start_time, stop_time, elapsed_time;
    int max_iterations = 4000;
    int iteration = 1;
    MPI_Status status;              // status returned by MPI calls

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_PE_num);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);
    

    // Calculate ring neighbors
    next_PE = (my_PE_num + 1) % npes;
    prev_PE = (my_PE_num - 1 + npes) % npes;
    
    // PE 0 asks for input, default has been set to 4000
    // if(my_PE_num == 0) {
    //     printf("Maximum iterations [100-4000]?\n");
    //     fflush(stdout); // Not always necessary, but can be helpful
    //     scanf("%d", &max_iterations);
    // }
    MPI_Bcast(&max_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (my_PE_num==0) gettimeofday(&start_time,NULL);

    int rows_per_process = ROWS_GLOBAL / npes; // Dynamically Calculate local dimensions
    int ghost_rows = ROWS_GLOBAL % npes;
    int ROWS = rows_per_process + (my_PE_num < ghost_rows ? 1 : 0); // for even distribution of the ghost_rows in case the rows are not exactly divisible by process X.

    // Dynamically allocate memory space
    double **Temperature = (double **)malloc((ROWS + 2) * sizeof(double *));
    double **Temperature_last = (double **)malloc((ROWS + 2) * sizeof(double *));
    for (int i = 0; i < ROWS + 2; i++) {
        Temperature[i] = (double *)malloc((COLUMNS + 2) * sizeof(double));
        Temperature_last[i] = (double *)malloc((COLUMNS + 2) * sizeof(double));
    }
    // Optimized: Single contiguous memory allocation
    double (*Temperature)[COLUMNS+2] = malloc((ROWS+2) * (COLUMNS+2) * sizeof(double));

    initialze_circular(ROWS, npes, my_PE_num, Temperature_last);

    while (dt_global > MAX_TEMP_ERROR && iteration <= max_iterations) {
        // // Main calculation: average four neighbors
        // for (int i = 1; i <= ROWS; i++) {
        //     for (int j = 1; j <= COLUMNS; j++) {
        //         Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
        //                                     Temperature_last[i][j+1] + Temperature_last[i][j-1]);
        //     }
        // }

        //only send/recv more than 1 processors
        // if (npes > 1) {
            // COMMUNICATION PHASE: a circular ring communication (avoiding deadlock)
            // Each PE sends its bottom row to next_PE and receives top ghost row from prev_PE
        //     if (my_PE_num % 2 == 0) {
        //         if (verbose) printf("PE %d sending bottom row to PE %d\n", my_PE_num, next_PE);
        //         MPI_Send(&Temperature[ROWS][1], COLUMNS, MPI_DOUBLE, next_PE, DOWN, MPI_COMM_WORLD);
        //         if (verbose) printf("PE %d receiving top ghost row from PE %d\n", my_PE_num, prev_PE);
        //         MPI_Recv(&Temperature[0][1], COLUMNS, MPI_DOUBLE, prev_PE, DOWN, MPI_COMM_WORLD, &status);
        //     } else {
        //         if (verbose) printf("PE %d receiving top ghost row from PE %d\n", my_PE_num, prev_PE);
        //         MPI_Recv(&Temperature[0][1], COLUMNS, MPI_DOUBLE, prev_PE, DOWN, MPI_COMM_WORLD, &status);
        //         if (verbose) printf("PE %d sending bottom row to PE %d\n", my_PE_num, next_PE);
        //         MPI_Send(&Temperature[ROWS][1], COLUMNS, MPI_DOUBLE, next_PE, DOWN, MPI_COMM_WORLD);
        //     }

        //     // Each PE sends its top row to prev_PE and receives bottom ghost row from next_PE
        //     if (my_PE_num % 2 == 0) {
        //         if (verbose) printf("PE %d sending top row to PE %d\n", my_PE_num, prev_PE);
        //         MPI_Send(&Temperature[1][1], COLUMNS, MPI_DOUBLE, prev_PE, UP, MPI_COMM_WORLD);
        //         if (verbose) printf("PE %d receiving bottom ghost row from PE %d\n", my_PE_num, next_PE);
        //         MPI_Recv(&Temperature[ROWS+1][1], COLUMNS, MPI_DOUBLE, next_PE, UP, MPI_COMM_WORLD, &status);
        //     } else {
        //         if (verbose) printf("PE %d receiving bottom ghost row from PE %d\n", my_PE_num, next_PE);
        //         MPI_Recv(&Temperature[ROWS+1][1], COLUMNS, MPI_DOUBLE, next_PE, UP, MPI_COMM_WORLD, &status);
        //         if (verbose) printf("PE %d sending top row to PE %d\n", my_PE_num, prev_PE);
        //         MPI_Send(&Temperature[1][1], COLUMNS, MPI_DOUBLE, prev_PE, UP, MPI_COMM_WORLD);
        //     }
        // }

        // Phase 1: Post all non-blocking operations
        MPI_Request requests[4];
        int req_count = 0;
        MPI_Isend(&Temperature[ROWS][1], COLUMNS, MPI_DOUBLE, next_PE, DOWN, MPI_COMM_WORLD, &requests[req_count++]);
        MPI_Irecv(&Temperature[0][1], COLUMNS, MPI_DOUBLE, prev_PE, DOWN, MPI_COMM_WORLD, &requests[req_count++]);

        // Phase 2: Calculate interior points (overlap!)
        for(i = 2; i < ROWS; i++) {
            for(j = 1; j <= COLUMNS; j++) {
                Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
                                            Temperature_last[i][j+1] + Temperature_last[i][j-1]);
            }
        }

        // Compute local max difference and update Temperature_last
        //dt = 0.0;
        for (int i = 1; i <= ROWS; i++) {
            for (int j = 1; j <= COLUMNS; j++) {            
                // Optimized: Pointer swapping
                double (*temp_ptr)[COLUMNS+2] = Temperature_last;
                Temperature_last = Temperature;
                Temperature = temp_ptr;            
            }
        }
        MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        // Find global dt
        // Optimized: Single operation
        //MPI_Allreduce(&dt, &dt_global, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        // periodically print test values - only for PE in lower corner
        if((iteration % 100) == 0) {
            if (my_PE_num == npes-1){
                track_progress(iteration, ROWS, Temperature);
            }
        }   

        iteration++;
    }
    // PE 0 finish timing and output values
    if (my_PE_num==0){
        gettimeofday(&stop_time,NULL);
        timersub(&stop_time, &start_time, &elapsed_time);
        printf("\n================================= result ===================================");
        printf("\nMax error at iteration %d was %f\n", iteration-1, dt_global);
        printf("Total time was %f seconds.\n", elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);
        printf("============================================================================\n");
    }

    // Free memory after all of the communication has finished
    for (int i = 0; i < ROWS + 2; i++) {
        free(Temperature[i]);
        free(Temperature_last[i]);
    }
    free(Temperature);
    free(Temperature_last);

    MPI_Finalize();
    return 0;
}
