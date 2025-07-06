/****************************************************************
 * Complete Working 1D Linear Laplace MPI Solver
 * Project: CI Pathway Summer 2025 
 * Course: Parallel Programming 
 * Assignment: Distributed Memory Parallelism                                                                 
 * Original Author: John Urbanic, PSC 2014
 * Modified by: Hochan Son, UCLA, Statistics
 * Date: 2025-07-05
 * 
 * COMPLETE WORKING VERSION with:
 * - Correct boundary conditions (matches original exactly)
 * - Linear topology (no circular communication)
 * - Dynamic process count support
 * - Proper convergence checking
 * - Fixed ghost cell communication
 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define COLUMNS      1000
#define ROWS_GLOBAL  1000
#define MAX_TEMP_ERROR 0.01

// Communication tags
#define DOWN     100
#define UP       101   

// Global variables for temperature arrays
double **Temperature;
double **Temperature_last;

// Function prototypes
void initialize(int npes, int my_PE_num, int my_rows);
void track_progress(int iteration, int my_rows, int npes, int my_PE_num);

int main(int argc, char *argv[]) {
    int i, j;
    int max_iterations;
    int iteration = 1;
    double dt;
    struct timeval start_time, stop_time, elapsed_time;

    int npes;                // number of PEs
    int my_PE_num;           // my PE number
    double dt_global = 100;  // delta t across all PEs
    MPI_Status status;       // status returned by MPI calls

    // MPI startup routines
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_PE_num);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);

    // Calculate dynamic local dimensions
    int rows_per_process = ROWS_GLOBAL / npes;
    int extra_rows = ROWS_GLOBAL % npes;
    
    // Handle uneven division
    int my_rows = rows_per_process;
    if (my_PE_num < extra_rows) {
        my_rows++;  // First 'extra_rows' processes get one extra row
    }

    if (my_PE_num == 0) {
        printf("=== Complete Working 1D Linear Laplace MPI Solver ===\n");
        printf("Running with %d processes\n", npes);
        printf("Grid size: %d x %d\n", ROWS_GLOBAL, COLUMNS);
        printf("Process 0 managing %d rows\n", my_rows);
    }

    // Dynamic memory allocation for 2D arrays
    Temperature = malloc((my_rows + 2) * sizeof(double*));
    Temperature_last = malloc((my_rows + 2) * sizeof(double*));
    
    for(i = 0; i < my_rows + 2; i++) {
        Temperature[i] = malloc((COLUMNS + 2) * sizeof(double));
        Temperature_last[i] = malloc((COLUMNS + 2) * sizeof(double));
    }

    // PE 0 asks for input
    if(my_PE_num == 0) {
        printf("Maximum iterations [100-4000]?\n");
        fflush(stdout);
        scanf("%d", &max_iterations);
    }

    // Broadcast max iterations to other PEs
    MPI_Bcast(&max_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_PE_num == 0) gettimeofday(&start_time, NULL);

    // Initialize boundary conditions
    initialize(npes, my_PE_num, my_rows);

    // Main iteration loop
    while (dt_global > MAX_TEMP_ERROR && iteration <= max_iterations) {

        // === MAIN CALCULATION: average my four neighbors ===
        for(i = 1; i <= my_rows; i++) {
            for(j = 1; j <= COLUMNS; j++) {
                Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
                                            Temperature_last[i][j+1] + Temperature_last[i][j-1]);
            }
        }

        // === COMMUNICATION PHASE: send ghost rows for next iteration ===

        // Send bottom real row down
        if(my_PE_num != npes-1) {  // unless we are bottom PE
            MPI_Send(&Temperature[my_rows][1], COLUMNS, MPI_DOUBLE, my_PE_num+1, DOWN, MPI_COMM_WORLD);
        }

        // Receive the bottom row from above into our top ghost row
        if(my_PE_num != 0) {  // unless we are top PE
            MPI_Recv(&Temperature_last[0][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, DOWN, MPI_COMM_WORLD, &status);
        }

        // Send top real row up
        if(my_PE_num != 0) {  // unless we are top PE
            MPI_Send(&Temperature[1][1], COLUMNS, MPI_DOUBLE, my_PE_num-1, UP, MPI_COMM_WORLD);
        }

        // Receive the top row from below into our bottom ghost row
        if(my_PE_num != npes-1) {  // unless we are bottom PE
            MPI_Recv(&Temperature_last[my_rows+1][1], COLUMNS, MPI_DOUBLE, my_PE_num+1, UP, MPI_COMM_WORLD, &status);
        }

        // === CONVERGENCE CHECK ===
        dt = 0.0;
        for(i = 1; i <= my_rows; i++) {
            for(j = 1; j <= COLUMNS; j++) {
                dt = fmax(fabs(Temperature[i][j] - Temperature_last[i][j]), dt);
                Temperature_last[i][j] = Temperature[i][j];
            }
        }

        // Find global dt                                                        
        MPI_Reduce(&dt, &dt_global, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Bcast(&dt_global, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Periodically print test values - only for PE in lower corner
        if((iteration % 100) == 0) {
            if (my_PE_num == npes-1) {
                track_progress(iteration, my_rows, npes, my_PE_num);
            }
            if (my_PE_num == 0) {
                printf("Iteration %d: dt_global = %f\n", iteration, dt_global);
            }
        }

        iteration++;
    }

    // Synchronize for accurate timing
    MPI_Barrier(MPI_COMM_WORLD);

    // PE 0 finish timing and output values
    if (my_PE_num == 0) {
        gettimeofday(&stop_time, NULL);
        timersub(&stop_time, &start_time, &elapsed_time);

        printf("\n============== RESULTS ==============\n");
        if (dt_global <= MAX_TEMP_ERROR) {
            printf("✅ CONVERGED after %d iterations\n", iteration-1);
        } else {
            printf("❌ Did NOT converge after %d iterations\n", iteration-1);
        }
        printf("Final error: %f\n", dt_global);
        printf("Total time: %f seconds\n", elapsed_time.tv_sec + elapsed_time.tv_usec/1000000.0);
        printf("====================================\n");
    }

    // Free memory
    for(i = 0; i < my_rows + 2; i++) {
        free(Temperature[i]);
        free(Temperature_last[i]);
    }
    free(Temperature);
    free(Temperature_last);

    MPI_Finalize();
    return 0;
}

void initialize(int npes, int my_PE_num, int my_rows) {
    double tMin, tMax;  // Local boundary limits
    int i, j;

    // Initialize all points to 0.0
    for(i = 0; i <= my_rows + 1; i++) {
        for (j = 0; j <= COLUMNS + 1; j++) {
            Temperature_last[i][j] = 0.0;
        }
    }

    // ✅ CRITICAL: Correct boundary condition calculation
    // This EXACTLY matches the original working version
    tMin = (my_PE_num) * 100.0 / npes;
    tMax = (my_PE_num + 1) * 100.0 / npes;

    // Left and right boundaries
    for (i = 0; i <= my_rows + 1; i++) {
        Temperature_last[i][0] = 0.0;  // Left boundary = 0°C
        Temperature_last[i][COLUMNS+1] = tMin + ((tMax - tMin) / my_rows) * i;  // Right boundary: linear gradient
    }

    // Top boundary (PE 0 only)
    if (my_PE_num == 0) {
        for (j = 0; j <= COLUMNS + 1; j++) {
            Temperature_last[0][j] = 0.0;  // Top boundary = 0°C
        }
    }

    // Bottom boundary (Last PE only)
    if (my_PE_num == npes - 1) {
        for (j = 0; j <= COLUMNS + 1; j++) {
            Temperature_last[my_rows + 1][j] = (100.0 / COLUMNS) * j;  // Bottom: 0°C to 100°C
        }
    }

    printf("Process %d: initialized boundaries (tMin=%.1f, tMax=%.1f, rows=%d)\n", 
           my_PE_num, tMin, tMax, my_rows);
}

void track_progress(int iteration, int my_rows, int npes, int my_PE_num) {
    int i;
    
    printf("---------- Iteration number: %d ------------\n", iteration);

    // Calculate global coordinates for display
    // This matches the original algorithm exactly
    int rows_per_process = ROWS_GLOBAL / npes;
    int extra_rows = ROWS_GLOBAL % npes;
    
    int global_start_row = my_PE_num * rows_per_process;
    if (my_PE_num < extra_rows) {
        global_start_row += my_PE_num;
    } else {
        global_start_row += extra_rows;
    }

    // Output global coordinates so user doesn't have to understand decomposition
    for(i = 5; i >= 0; i--) {
        if (my_rows - i > 0) {
            int global_row = global_start_row + (my_rows - i);
            printf("[%d,%d]: %5.2f  ", global_row, COLUMNS - i, Temperature[my_rows - i][COLUMNS - i]);
        }
    }
    printf("\n");
}