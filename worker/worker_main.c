#include <stdlib.h>
#include <stdio.h>
#include <mpi/mpi.h>
#include "../common/lib.h"

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    MPI_Comm parent_comm;
    MPI_Comm_get_parent(&parent_comm);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Worker [%d]: Started...\n", rank);
    struct configuration_t config;
    read_configuration_from_argv(&config, argv);
    printf("Worker [%d]: lines = %d, columns = %d, split threshold = %d, max value = %d\n",
           rank,
           config.number_of_lines,
           config.number_of_columns,
           config.split_threshold,
           config.max_value
    );


    int *line = malloc(config.number_of_columns * sizeof(int));
    if (line == NULL) {
        fprintf(stderr, "Worker [%d]: Could not malloc buffer...\n");
        exit(1);
    }
    int parent_rank = 0;

    // todo: how to show an integer to distinguish between comms?
    printf("Worker [%d]: Receiving line from parent...\n", rank);
    MPI_Recv(line, config.number_of_columns, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm, MPI_STATUS_IGNORE);
    printf("Worker [%d]: Received line from parent...\n", rank);

    int sum = sum_of_slice(line, config.number_of_columns);

    printf("Worker [%d]: Sending sum of line to parent...\n", rank);
    MPI_Send(&sum, 1, MPI_INT, parent_rank, 0, parent_comm);
    printf("Worker [%d]: Sent sum of line to parent...\n", rank);

    MPI_Finalize();
    free_pointer((void **) &line);
    return 0;
}