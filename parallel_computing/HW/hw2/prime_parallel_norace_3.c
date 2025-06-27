# include <stdlib.h>
# include <stdio.h>
#include <omp.h>

#define MAX_THREADS 32

/*
  parallelism: private variable with reduction method 
  compiler: private(i,j,not_primes) and reduction(+:not_primes)
  Speed: fast
*/

int main ( int argc, char *argv[] ){

  int n = 500000;
  int not_primes=0; // global_shared_variables
  int i,j;
  
  // Set number of threads at runtime
  int num_threads = MAX_THREADS;
  if (omp_get_max_threads() < MAX_THREADS) {
      num_threads = omp_get_max_threads();
  }
  omp_set_num_threads(num_threads);
  printf("Running with %d OpenMP threads\n", num_threads);

  #pragma omp for private(i,j,not_primes) reduction(+:not_primes)
  for ( i = 2; i <= n; i++ ){
    for ( j = 2; j < i; j++ ){
      if ( i % j == 0 ){
        not_primes++;
        break;
      }
    }
  }
  printf("Primes: %d\n", n - not_primes);
}

