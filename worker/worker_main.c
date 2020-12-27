#include <stdlib.h>
#include <stdio.h>
#include <mpi/mpi.h>
#include "../common/lib.h"

int main(int argc, char *argv[]) {
    int number_of_lines = 10;
    int number_of_columns = 10;
    int split_threshold = 1000;

    MPI_Init(&argc, &argv);

    MPI_Comm parent_comm;
    MPI_Comm_get_parent(&parent_comm);

    int rank, number_of_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

    printf("Worker [%d]: Started...\n", rank);

    int *line = malloc(number_of_columns * sizeof(int));
    int parent_rank = 0;

    // todo: how to show an integer to distinguish between comms?
    printf("Worker [%d]: Receiving line from parent...\n", rank);
    MPI_Recv(line, number_of_columns, MPI_INT, parent_rank, MPI_ANY_TAG, parent_comm, MPI_STATUS_IGNORE);
    printf("Worker [%d]: Received line from parent...\n", rank);

    int sum = sum_of_slice(line, number_of_columns);

    printf("Worker [%d]: Sending sum of line to parent...\n", rank);
    MPI_Send(&sum, 1, MPI_INT, parent_rank, 0, parent_comm);
    printf("Worker [%d]: Sent sum of line to parent...\n", rank);

    MPI_Finalize();
    free_pointer((void **) &line);
    return 0;
}