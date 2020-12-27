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
    // todo: parse arguments from commandline
    int number_of_lines = 10;
    int number_of_columns = 10;
    int split_threshold = 1000;

    int *matrix = create_random_rectangular_matrix(number_of_lines, number_of_columns, 10000);

    MPI_Init(NULL, NULL);

    printf("[Root] Spawning children...\n");
    MPI_Comm children_comm;
    MPI_Comm_spawn("./worker_proc",
                   argv + 1, // todo: careful here, might need a null string at the end
                   number_of_lines,
                   MPI_INFO_NULL,
                   0,
                   MPI_COMM_SELF,
                   &children_comm,
                   MPI_ERRCODES_IGNORE
    );
    printf("[Root] Spawned children\n");

    for (int line = 0; line < number_of_lines; ++line) {
        MPI_Send(matrix + line * number_of_columns, number_of_columns, MPI_INT, line, 0, children_comm);
    }

    // Wait for results from all children
    int *received_line_sums = calloc(number_of_lines, sizeof(int));
    for (int line = 0; line < number_of_lines; ++line) {
        MPI_Recv(&received_line_sums[line], 1, MPI_INT, line, MPI_ANY_TAG, children_comm, MPI_STATUS_IGNORE);
    }

    // Check results
    for (int line = 0; line < number_of_lines; ++line) {
        int line_sum = sum_of_slice(matrix + line * number_of_columns, number_of_columns);
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