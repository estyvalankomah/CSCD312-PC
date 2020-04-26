
#include <mpi.h> 
#include <stdio.h>  
  
int main(int argc, char* argv[]) 
{ 

int message, world_rank, world_size;

MPI_Init(&argc, &argv);

MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
MPI_Comm_size(MPI_COMM_WORLD, &world_size);

if (world_rank != 0) {
    MPI_Recv(&message, 1, MPI_INT, world_rank - 1, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received message '%d' from process %d\n",
           world_rank, message, world_rank - 1);
} else {
    message = 2;
}
MPI_Send(&message, 1, MPI_INT, (world_rank + 1) % world_size,
         0, MPI_COMM_WORLD);


if (world_rank == 0) {
    MPI_Recv(&message, 1, MPI_INT, world_size - 1, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received message '%d' from process %d\n",
           world_rank, message, world_size - 1);
}

    MPI_Finalize(); 
  
    return 0; 
} 

