# include <stdlib.h>
# include <stdio.h>
# include <omp.h>
# include <math.h>

#define MAX_THREADS 32

/*
  parallelism: private variable with reduction + algorithm optimization
  compiler: private(i,j) reduction(+:not_primes)
  Speed: fastest O(nlogn)
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

  #pragma omp parallel for private(i,j) reduction(+:not_primes)
  for ( i = 2; i <= n; i++ ){
    if (i == 2) continue;
    if (i % 2 == 0) { not_primes++; continue; }

    int sqrt_i = (int)sqrt(i);
    for (j = 3; j <= sqrt_i; j += 2) {
      if ( i % j == 0 ){
        not_primes++;
        break;
      }
    }
  }
  printf("Primes: %d\n", n - not_primes);
}