#!/bin/bash

################################################
# Project: CI Pathway
# Title: Assignment-3
# Objective: 
# Added iterative Laplace solver logic: The code now iteratively updates the temperature grid using the average of four neighbors, checking for convergence with a global maximum difference (dt_global).
# Implemented robust circular (ring) communication: Each PE exchanges boundary rows with its neighbors using a deadlock-free pattern, supporting any number of processes.
# Improved boundary initialization: The initialze_circular function sets left/right boundaries for all PEs, and top/bottom boundaries for the first/last PE, ensuring proper boundary conditions.
# Added periodic progress reporting: The code prints progress every 100 iterations from the last PE, showing representative grid values.
# Added timing and result summary: The code measures and reports total runtime and final error on PE 0.
# Memory management: Dynamic allocation and cleanup of 2D arrays for temperature grids.
# Verbose control: Communication messages are printed only if the verbose flag is enabled.
# Date: 2025-06-30
# Author: Hochan Son
# Email: ohsono@gmail.com or hochanson@g.ucla.edu
################################################
# Output file
output_file="hw3_result.txt"

# Clear previous results
> ${output_file}

#srun --account=becs-delta-cpu --partition=cpu-interactive --nodes=1 \
#  --tasks=8 --tasks-per-node=8 --pty bash

# Add header with system information
echo "=== Basic System Info ===" > ${output_file}
echo "Date: $(date)" >> ${output_file}
echo "System: $(uname -a)" >> ${output_file}
echo "CPU Info: $(lscpu | grep 'Model name' | sed -r 's/Model name:\s{1,}//g')" >> ${output_file}
echo "=========================" >> ${output_file}

# Array of PE counts to test
pe_counts=(1 4 8)


echo "!!!!STARTING MPI PROCESS TEST - original !!!!">> ${output_file}
# run tests for Parallel process
for pe in "${pe_counts[@]}"
do
    echo "Running with ${pe} pe..."
    echo "=== Test with ${pe} pe ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set pe count and run program
    TIMEFORMAT='%3R'
    runtime=$( { time echo 4000 | mpirun -n ${pe} laplace_mpi.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "MPI Parallel Process: Testing complete. Results saved in ${output_file}"
# end of the parallel process test



echo "!!!!STARTING MPI PROCESS TEST - 1d optimized!!!!">> ${output_file}
# run tests for Parallel process
for pe in "${pe_counts[@]}"
do
    echo "Running with ${pe} pe..."
    echo "=== Test with ${pe} pe ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set pe count and run program
    TIMEFORMAT='%3R'
    runtime=$( { time mpirun -n ${pe} hw3_laplace_mpi_2.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "MPI Parallel Process: Testing complete. Results saved in ${output_file}"
# end of the parallel process test


echo "!!!!STARTING MPI PROCESS TEST -2d optimized !!!!">> ${output_file}
# run tests for Parallel process
for pe in "${pe_counts[@]}"
do
    echo "Running with ${pe} pe..."
    echo "=== Test with ${pe} pe ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set pe count and run program
    TIMEFORMAT='%3R'
    runtime=$( { time mpirun -n ${pe} hw3_laplace_mpi_4.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "MPI Parallel Process: Testing complete. Results saved in ${output_file}"
# end of the parallel process test