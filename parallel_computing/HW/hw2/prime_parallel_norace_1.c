# include <stdlib.h>
# include <stdio.h>
#include <omp.h>

#define MAX_THREADS 32

/*
  parallelism: using atomic operation
  compiler: private(i,j) -> atomic operation
  Speed: slow
*/

int main ( int argc, char *argv[] ){

  int n = 500000;
  int not_primes=0;
  int i,j;
      
  // Set number of threads at runtime
  int num_threads = MAX_THREADS;
  if (omp_get_max_threads() < MAX_THREADS) {
      num_threads = omp_get_max_threads();
  }
  omp_set_num_threads(num_threads);
  printf("Running with %d OpenMP threads\n", num_threads);

  // parallel running reduction with serial atomic counts
  #pragma omp parallel for private(i,j)
  for ( i = 2; i <= n; i++ ){
    for ( j = 2; j < i; j++ ){
      if ( i % j == 0 ){
        #pragma omp atomic
        not_primes++;
        break;
      }
    }
  }

  printf("Primes: %d\n", n - not_primes);

}

