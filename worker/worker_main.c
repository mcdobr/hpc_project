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
        fprintf(stderr, "Worker [%d]: Could not malloc buffer...\n", rank);
        exit(1);
    }


    // Receive buffer size from parent
    int buffer_length;
    printf("Worker [%d]: Receiving buffer size from parent...\n", rank);
    MPI_Recv(&buffer_length, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm, MPI_STATUS_IGNORE);
    printf("Worker [%d]: Received buffer size %d from parent...\n", rank, buffer_length);

    // Receive buffer from parent
    // todo: how to show an integer to distinguish between comms?
    printf("Worker [%d]: Receiving line from parent...\n", rank);
    MPI_Recv(line, config.number_of_columns, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, parent_comm, MPI_STATUS_IGNORE);
    printf("Worker [%d]: Received line from parent...\n", rank);

//    debug_print(line, buffer_length);

    int sum;
    if (buffer_length < config.split_threshold) {
        sum = sum_of_slice(line, buffer_length);
    } else {
        int first_buffer_length = buffer_length / 2;
        int second_buffer_length = buffer_length / 2 + (buffer_length % 2);

        // spawn 2 children
        printf("Worker [%d] Spawning children...\n", rank);
        MPI_Comm children_comm;
        MPI_Comm_spawn("./worker_proc",
                       argv + 1,
                       2,
                       MPI_INFO_NULL,
                       0, // todo: need to check this
                       MPI_COMM_SELF,
                       &children_comm,
                       MPI_ERRCODES_IGNORE
        );
        printf("Worker [%d] Spawned children\n", rank);

        // send buffer lengths to those children
        MPI_Send(&first_buffer_length, 1, MPI_INT, 0, 0, children_comm);
        MPI_Send(&second_buffer_length, 1, MPI_INT, 1, 0, children_comm);

        // send buffers to those children
        MPI_Send(line, first_buffer_length, MPI_INT, 0, 0, children_comm);
        MPI_Send(line + first_buffer_length, second_buffer_length, MPI_INT, 1, 0, children_comm);

        // receive sums from those two children
        int first_partial_sum, second_partial_sum;
        MPI_Recv(&first_partial_sum, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, children_comm, MPI_STATUS_IGNORE);
        MPI_Recv(&second_partial_sum, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, children_comm, MPI_STATUS_IGNORE);

        printf("Received from spawned children %d and %d\n", first_partial_sum, second_partial_sum);

        sum = first_partial_sum + second_partial_sum;
    }

    // Send result back to parent
    int parent_rank = 0;
    printf("Worker [%d]: Sending sum of line to parent...\n", rank);
    MPI_Send(&sum, 1, MPI_INT, parent_rank, 0, parent_comm);
    printf("Worker [%d]: Sent sum of line to parent...\n", rank);

    MPI_Finalize();
    free_pointer((void **) &line);
    return 0;
}