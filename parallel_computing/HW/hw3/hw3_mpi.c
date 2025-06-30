#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv){
  int my_PE_num, npes;    //current PE, number of processes
  int next_PE, prev_PE;   //next PE number, previous PE number

  // Each process sends its rank * 100
  send_data = my_rank * 100;

  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_PE_num);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);

  next_PE = (my_PE_num + 1) % npes;
  prev_PE = ((my_PE_num - 1) + npes) % npes; // to avoid divid by zero error when my_PE_num - 1 = 0.

  printf("Process %d: next=%d, prev=%d, sending=%d\n", my_PE_num, next_PE, prev_PE, send_data);

  // Broadcast work to all processes (including PE_0)
  MPI_Bcast(&work_data, 1, MPI_INT, 0, MPI_COMM_WORLD);

  //Sender
  MPI_Send( &send_data, 1,MPI_INT, next_PE, 10,MPI_COMM_WORLD);
  
  //Reciever 
  MPI_Recv( &numbertoreceive, 1, MPI_INT, prev_PE, 10, MPI_COMM_WORLD, &status);
  
  MPI_Finalize();
  return 0;
}