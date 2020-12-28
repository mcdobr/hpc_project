// This implementation will only comply with POSIX
#include <stdio.h>
#include <mpi/mpi.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "../common/lib.h"

int main(int argc, char *argv[]) {
    srand(time(NULL));
    printf("[Root] started...\n");

    struct configuration_t config;
    read_configuration_from_argv(&config, argv);
    printf("[Root]: lines = %d, columns = %d, split threshold = %d, max value = %d\n",
           config.number_of_lines,
           config.number_of_columns,
           config.split_threshold,
           config.max_value
    );

    int *matrix = create_random_rectangular_matrix(config.number_of_lines, config.number_of_columns, config.max_value);

    MPI_Init(NULL, NULL);

    printf("[Root] Spawning children...\n");
    MPI_Comm children_comm;
    MPI_Comm_spawn("./worker_proc",
                   argv + 1,
                   config.number_of_lines,
                   MPI_INFO_NULL,
                   0,
                   MPI_COMM_SELF,
                   &children_comm,
                   MPI_ERRCODES_IGNORE
    );
    printf("[Root] Spawned children\n");

    MPI_Request send_buffer_sizes_requests[config.number_of_lines];
    for (int line = 0; line < config.number_of_lines; ++line) {
        printf("Sending buffer size to child %d\n", line);
        MPI_Isend(&config.number_of_columns,
                  1,
                  MPI_INT,
                  line,
                  0,
                  children_comm,
                  &send_buffer_sizes_requests[line]
        );
    }
    MPI_Waitall(config.number_of_lines, send_buffer_sizes_requests, MPI_STATUSES_IGNORE);

    MPI_Request send_requests[config.number_of_lines];
    for (int line = 0; line < config.number_of_lines; ++line) {
        printf("Sending line to child %d\n", line);
        MPI_Isend(matrix + line * config.number_of_columns,
                  config.number_of_columns,
                  MPI_INT,
                  line,
                  0,
                  children_comm,
                  &send_requests[line]
        );
    }

    // Wait for results from all children
    int *received_line_sums = calloc(config.number_of_lines, sizeof(int));
    for (int line = 0; line < config.number_of_lines; ++line) {
        MPI_Recv(&received_line_sums[line], 1, MPI_INT, line, MPI_ANY_TAG, children_comm, MPI_STATUS_IGNORE);
    }

    // Check results
    for (int line = 0; line < config.number_of_lines; ++line) {
        int line_sum = sum_of_slice(matrix + line * config.number_of_columns, config.number_of_columns);
        assert(line_sum == received_line_sums[line]);
        printf("[Root]: Received for line %d the value %d and locally it was computed as %d\n",
               line,
               line_sum,
               received_line_sums[line]
        );
    }
    printf("[Root]: All processes have terminated and have returned CORRECT sums\n");

    // Clean up
    MPI_Finalize();
    free_pointer((void **) &matrix);
    return 0;
}