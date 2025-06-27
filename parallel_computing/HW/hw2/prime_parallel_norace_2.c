# include <stdlib.h>
# include <stdio.h>
#include <omp.h>

#define MAX_THREADS 32

/*
  parallelism: manual reduction with scheduler with local variables
  compiler: private(i,j) -> schedule(static) -> critical
  Speed: normal
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

  #pragma omp parallel private(i,j,local_not_primes)
  {
    int local_not_primes = 0;

    #pragma omp for schedule(static)
    for ( i = 2; i <= n; i++ ){
      for ( j = 2; j < i; j++ ){
        if ( i % j == 0 ){
          local_not_primes++;
          break;
        }
      }
    }

    #pragma omp critical
    {
      not_primes += local_not_primes;
    }
  }
  printf("Primes: %d\n", n - not_primes);
}

