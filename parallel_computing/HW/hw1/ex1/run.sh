#!/bin/bash

################################################
# Project: CI Pathway Parallel computing
# Title: Assignment-1
# Objective: 
# Date: 2025-06-17
# Author: Hochan Son
# Email: ohsono@gmail.com or hochanson@g.ucla.edu
################################################
# Output file
output_file="result.txt"
max_itr=4000
# Clear previous results
> ${output_file}

# Add header with system information
echo "=== Basic System Info ===" > ${output_file}
echo "Date: $(date)" >> ${output_file}
echo "System: $(uname -a)" >> ${output_file}
echo "CPU Info: $(lscpu | grep 'Model name' | sed -r 's/Model name:\s{1,}//g')" >> ${output_file}
echo "=========================" >> ${output_file}

# Array of thread counts to test
thread_counts=(1 8 32)


# First run tests for serial process
echo "!!!!STARTING SERIAL PROCESS TEST!!!!"  >> ${output_file}
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time echo ${max_itr}|./laplace_s.out >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Serial Process: Testing complete. Results saved in ${output_file}"
# end of the serial process test


# Second run tests for omp process
echo "!!!!STARTING OMP PROCESS TEST!!!!"  >> ${output_file}
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time echo ${max_itr}|./laplace_o.out >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "OMP Process: Testing complete. Results saved in ${output_file}"
# end of the OMP process test

# Third run tests for Enhanced Parallel process (RED/BLACK)
echo "!!!!STARTING Enhanced (RED/BLACK) PARALLEL PROCESS TEST!!!!" >> ${output_file}
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time echo ${max_itr}| ./laplace_p.out >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Enhanced(RED/BLACK) Parallel Process: Testing complete. Results saved in ${output_file}"
# end of the parallel process test
