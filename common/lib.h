#ifndef HPC_PROJECT_LIB_H
#define HPC_PROJECT_LIB_H

#include <assert.h>

int *create_random_rectangular_matrix(const int number_of_lines, const int number_of_columns, const int max_value);

int *create_random_array(const int number_of_elements, const int max_value);

int sum_of_slice(const int *const pointer, int slice_size);

void free_pointer(void **ptr);

int *create_random_rectangular_matrix(const int number_of_lines, const int number_of_columns, const int max_value) {
    return create_random_array(number_of_lines * number_of_columns, max_value);
}

int *create_random_array(const int number_of_elements, const int max_value) {
    int *ptr = malloc(number_of_elements * sizeof(int));
    for (size_t i = 0; i < number_of_elements; ++i) {
        ptr[i] = rand() % max_value;
    }
    return ptr;
}

int sum_of_slice(const int *const pointer, int slice_size) {
    int sum = 0;
    for (int iterator = 0; iterator < slice_size; ++iterator) {
        sum += pointer[iterator];
    }
    return sum;
}

void free_pointer(void **ptr) {
    assert(ptr != NULL);
    assert(*ptr != NULL);
    free(*ptr);
    *ptr = NULL;
}

#endif //HPC_PROJECT_LIB_H
