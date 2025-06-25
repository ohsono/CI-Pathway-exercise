#!/bin/bash

############################################################
# Project: CI Pathway Parallel computing
# Title: Assignment-1 ex-2 mpi vs serial
# Objective: 
# Date: 2025-06-25
# Author: Hochan Son
# Email: ohsono@gmail.com (CI path) or hochanson@g.ucla.edu
############################################################
# Output file
output_file="ex2_result.txt"
max_itr=4000
# Clear previous results
> ${output_file}


# Grab multiple cores on a compute node
#srun --account=becs-delta-cpu --partition=cpu-interactive --nodes=1 --tasks=4 --tasks-per-node=4 --pty bash

# Add header with system information
echo "=== Basic System Info ===" > ${output_file}
echo "Date: $(date)" >> ${output_file}
echo "System: $(uname -a)" >> ${output_file}
echo "CPU Info: $(lscpu | grep 'Model name' | sed -r 's/Model name:\s{1,}//g')" >> ${output_file}
echo "=========================" >> ${output_file}

# mpicc ./laplace_serial.c -o laplace_s.out
# mpicc ./laplace_mpi.c -o laplace_m.out

# Array of thread counts to test
thread_counts=(1 4)

# First run tests for serial process
echo "!!!!STARTING SERIAL process TEST!!!!"  >> ${output_file}
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time echo ${max_itr} | mpirun -n ${threads} laplace_s.out >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Serial Process: Testing complete. Results saved in ${output_file}"
# end of the serial process test

# First run tests for mpi process
echo "!!!!STARTING MPI TEST!!!!"  >> ${output_file}
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time echo ${max_itr} | mpirun -n ${threads} laplace_m.out >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "MPI Process: Testing complete. Results saved in ${output_file}"
# end of the mpi process test

