/****************************************************************
 * Project: CI Pathway Summer 2025 
 * Course: Parallel Programming 
 * Assignment: Distributed Memory Parallelism - Optimized Version                                                           
 * Original Author: John Urbanic, PSC 2014
 * Author: Hochan Son, UCLA, Statistics
 * Date: 2025-07-05
 * 
 * Optimizations Applied:
 * 1. Contiguous memory allocation for better cache locality
 * 2. Non-blocking communication with computation overlap
 * 3. Pointer swapping instead of array copying
 * 4. AllReduce instead of Reduce+Bcast
 * 5. Proper load balancing with dynamic row distribution
 * 6. Fixed boundary communication pattern
 *
 * Expected Performance: 2-3x speedup over original version
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
#define VERBOSE 0

// Function prototypes
void initialize_optimized(int ROWS, int npes, int my_PE_num, double (*Temperature_last)[COLUMNS+2]);
void track_progress(int iteration, int ROWS, double (*Temperature)[COLUMNS+2]);

int main(int argc, char** argv) {
    int my_PE_num;                  // Current PE
    int npes;                       // number of PEs
    int next_PE, prev_PE;
    double dt, dt_global = 100;
    struct timeval start_time, stop_time, elapsed_time;
    int max_iterations = 4000;
    int iteration = 1;
    
    // MPI communication variables
    MPI_Request requests[4];        // for non-blocking communication
    int req_count;                  // number of active requests

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_PE_num);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);

    // Calculate ring neighbors
    next_PE = (my_PE_num + 1) % npes;
    prev_PE = (my_PE_num - 1 + npes) % npes;
    
    // Broadcast max iterations
    MPI_Bcast(&max_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (my_PE_num == 0) {
        printf("Running optimized version on %d processes\n", npes);
        printf("Grid size: %dx%d, Max iterations: %d\n", ROWS_GLOBAL, COLUMNS, max_iterations);
        gettimeofday(&start_time, NULL);
    }

    // Dynamic load balancing
    int rows_per_process = ROWS_GLOBAL / npes;
    int ghost_rows = ROWS_GLOBAL % npes;
    int ROWS = rows_per_process + (my_PE_num < ghost_rows ? 1 : 0);

    // OPTIMIZATION 1: Contiguous memory allocation for better cache locality
    double (*Temperature)[COLUMNS+2] = 
        (double (*)[COLUMNS+2])malloc((ROWS+2) * (COLUMNS+2) * sizeof(double));
    double (*Temperature_last)[COLUMNS+2] = 
        (double (*)[COLUMNS+2])malloc((ROWS+2) * (COLUMNS+2) * sizeof(double));
    
    if (!Temperature || !Temperature_last) {
        printf("PE %d: Memory allocation failed\n", my_PE_num);
        MPI_Finalize();
        exit(1);
    }

    // Initialize temperature arrays
    initialize_optimized(ROWS, npes, my_PE_num, Temperature_last);

    // Main computation loop
    while (dt_global > MAX_TEMP_ERROR && iteration <= max_iterations) {

        // OPTIMIZATION 2: Non-blocking communication with computation overlap
        req_count = 0;
        
        // Only communicate if we have more than 1 process
        if (npes > 1) {
            // Post non-blocking sends and receives for ghost row exchange
            if (my_PE_num != npes-1) {
                MPI_Isend(&Temperature_last[ROWS][1], COLUMNS, MPI_DOUBLE, 
                         next_PE, DOWN, MPI_COMM_WORLD, &requests[req_count++]);
            }
            if (my_PE_num != 0) {
                MPI_Irecv(&Temperature[0][1], COLUMNS, MPI_DOUBLE, 
                         prev_PE, DOWN, MPI_COMM_WORLD, &requests[req_count++]);
            }
            if (my_PE_num != 0) {
                MPI_Isend(&Temperature_last[1][1], COLUMNS, MPI_DOUBLE, 
                         prev_PE, UP, MPI_COMM_WORLD, &requests[req_count++]);
            }
            if (my_PE_num != npes-1) {
                MPI_Irecv(&Temperature[ROWS+1][1], COLUMNS, MPI_DOUBLE, 
                         next_PE, UP, MPI_COMM_WORLD, &requests[req_count++]);
            }
        }

        // Calculate interior points (can overlap with communication)
        // Interior points don't need ghost cells
        for (int i = 2; i < ROWS; i++) {
            for (int j = 1; j <= COLUMNS; j++) {
                Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
                                           Temperature_last[i][j+1] + Temperature_last[i][j-1]);
            }
        }

        // Wait for ghost cell exchange to complete
        if (npes > 1 && req_count > 0) {
            MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        }

        // Calculate boundary rows that need ghost cells
        // Top boundary row (row 1)
        for (int j = 1; j <= COLUMNS; j++) {
            Temperature[1][j] = 0.25 * (Temperature_last[2][j] + Temperature[0][j] +
                                       Temperature_last[1][j+1] + Temperature_last[1][j-1]);
        }
        
        // Bottom boundary row (row ROWS)
        for (int j = 1; j <= COLUMNS; j++) {
            Temperature[ROWS][j] = 0.25 * (Temperature[ROWS+1][j] + Temperature_last[ROWS-1][j] +
                                          Temperature_last[ROWS][j+1] + Temperature_last[ROWS][j-1]);
        }

        // Calculate convergence criterion
        dt = 0.0;
        for (int i = 1; i <= ROWS; i++) {
            for (int j = 1; j <= COLUMNS; j++) {
                dt = fmax(fabs(Temperature[i][j] - Temperature_last[i][j]), dt);
            }
        }

        // OPTIMIZATION 3: Pointer swapping instead of array copying
        double (*temp_ptr)[COLUMNS+2] = Temperature_last;
        Temperature_last = Temperature;
        Temperature = temp_ptr;

        // OPTIMIZATION 4: AllReduce instead of Reduce+Bcast
        MPI_Allreduce(&dt, &dt_global, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        // Periodically print progress
        if ((iteration % 100) == 0) {
            if (my_PE_num == npes-1) {
                track_progress(iteration, ROWS, Temperature_last);
            }
        }

        iteration++;
    }

    // Timing and results
    if (my_PE_num == 0) {
        gettimeofday(&stop_time, NULL);
        timersub(&stop_time, &start_time, &elapsed_time);
        printf("\n================================= RESULTS ===================================\n");
        printf("Max error at iteration %d was %f\n", iteration-1, dt_global);
        printf("Total time was %f seconds.\n", elapsed_time.tv_sec + elapsed_time.tv_usec/1000000.0);
        printf("Grid size: %dx%d, Processes: %d\n", ROWS_GLOBAL, COLUMNS, npes);
        printf("============================================================================\n");
    }

    // Clean up memory
    free(Temperature);
    free(Temperature_last);

    MPI_Finalize();
    return 0;
}

void initialize_optimized(int ROWS, int npes, int my_PE_num, double (*Temperature_last)[COLUMNS+2]) {
    // Initialize all cells to 0.0
    for (int i = 0; i <= ROWS + 1; i++) {
        for (int j = 0; j <= COLUMNS + 1; j++) {
            Temperature_last[i][j] = 0.0;
        }
    }

    // Calculate this PE's portion of the global boundary
    int rows_per_process = ROWS_GLOBAL / npes;
    int ghost_rows = ROWS_GLOBAL % npes;
    int my_start_row = 0;
    
    // Calculate starting row for this PE
    if (my_PE_num < ghost_rows) {
        my_start_row = my_PE_num * (rows_per_process + 1);
    } else {
        my_start_row = ghost_rows * (rows_per_process + 1) + 
                       (my_PE_num - ghost_rows) * rows_per_process;
    }

    // Set boundary conditions
    // Left boundary (always 0.0)
    for (int i = 0; i <= ROWS + 1; i++) {
        Temperature_last[i][0] = 0.0;
    }
    
    // Right boundary (temperature gradient)
    double t_min = my_start_row * 100.0 / ROWS_GLOBAL;
    double t_max = (my_start_row + ROWS) * 100.0 / ROWS_GLOBAL;
    for (int i = 0; i <= ROWS + 1; i++) {
        Temperature_last[i][COLUMNS+1] = t_min + ((t_max - t_min) / ROWS) * i;
    }

    // Top boundary (PE 0 only)
    if (my_PE_num == 0) {
        for (int j = 0; j <= COLUMNS + 1; j++) {
            Temperature_last[0][j] = 0.0;
        }
    }

    // Bottom boundary (last PE only)
    if (my_PE_num == npes-1) {
        for (int j = 0; j <= COLUMNS + 1; j++) {
            Temperature_last[ROWS+1][j] = 100.0 * j / COLUMNS;
        }
    }
}

void track_progress(int iteration, int ROWS, double (*Temperature)[COLUMNS+2]) {
    printf("---------- Iteration number: %d ------------\n", iteration);
    
    // Output representative values
    for (int i = 5; i >= 0; i--) {
        printf("[%d,%d]: %5.2f  ", ROWS_GLOBAL-i, COLUMNS-i, Temperature[ROWS-i][COLUMNS-i]);
    }
    printf("\n");
}