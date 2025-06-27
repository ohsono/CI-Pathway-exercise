#!/bin/bash

################################################
# Project: CI Pathway
# Title: Assignment-2
# Objective: 
#   1. prime number counting code to convert from serialized to parallel
#   2. run both serial/parallel testing foreach thread counts {1,4,8,16, and32}
#   3. log the output with the time difference
# Date: 2025-06-17
# Author: Hochan Son
# Email: ohsono@gmail.com or hochanson@g.ucla.edu
################################################
# Output file
output_file="hw2_result.txt"

# Clear previous results
> ${output_file}

# srun --account=becs-delta-cpu --partition=cpu-interactive \
#  --nodes=1 --cpus-per-task=32 --pty bash

# Add header with system information
echo "=== Basic System Info ===" > ${output_file}
echo "Date: $(date)" >> ${output_file}
echo "System: $(uname -a)" >> ${output_file}
echo "CPU Info: $(lscpu | grep 'Model name' | sed -r 's/Model name:\s{1,}//g')" >> ${output_file}
echo "=========================" >> ${output_file}

# Array of thread counts to test
thread_counts=(1 4 8 16 32)

echo "!!!!STARTING SERIAL PROCESS TEST!!!!"  >> ${output_file}
# First run tests for Serial process
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time ./prime_s.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Serial Process: Testing complete. Results saved in ${output_file}"
# end of the serial process test

echo "!!!!STARTING PARALLEL PROCESS TEST (fixed race:method-1)!!!!" >> ${output_file}
# Second run tests for Parallel process
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time ./prime_p_n_1.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Parallel Process(method-1): Testing complete. Results saved in ${output_file}"
# end of the parallel process test



echo "!!!!STARTING PARALLEL PROCESS TEST (fixed race:method-2)!!!!" >> ${output_file}
# Second run tests for Parallel process (fixed race)
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time ./prime_p_n_2.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Parallel Process (fixed race:method-2): Testing complete. Results saved in ${output_file}"
# end of the parallel process test (fixed race)


echo "!!!!STARTING PARALLEL PROCESS TEST (fixed race:method-3)!!!!" >> ${output_file}
# Second run tests for Parallel process (fixed race-2)
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time ./prime_p_n_3.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Parallel Process (fixed race:method-3): Testing complete. Results saved in ${output_file}"
# end of the parallel process test (fixed race-2)



echo "!!!!STARTING PARALLEL PROCESS TEST (fixed race:method-4)!!!!" >> ${output_file}
# Second run tests for Parallel process (fixed race-2)
for threads in "${thread_counts[@]}"
do
    echo "Running with ${threads} threads..."
    echo "=== Test with ${threads} threads ===" >> ${output_file}
    echo "Start time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    
    # Set thread count and run program
    export OMP_NUM_THREADS=${threads}
    TIMEFORMAT='%3R'
    runtime=$( { time ./prime_p_n_4.o >> ${output_file}; } 2>&1 )
    
    echo "End time: $(date '+%Y-%m-%d %H:%M:%S.%N')" >> ${output_file}
    echo "Total wall clock time: ${runtime} seconds" >> ${output_file}
    echo "----------------------------------------" >> ${output_file}
    echo "" >> ${output_file}
    
    # Add a small delay between runs
    sleep 1
done
echo "Parallel Process (fixed race:method-4): Testing complete. Results saved in ${output_file}"
# end of the parallel process test (fixed race-4)
