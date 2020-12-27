cmake_minimum_required(VERSION 3.17)
project(hpc_project C)

set(CMAKE_C_STANDARD 11)

find_package(MPI REQUIRED)

add_executable(root_proc root/root_main.c common/lib.h)
add_executable(worker_proc worker/worker_main.c)

target_link_libraries(root_proc MPI::MPI_C)
target_link_libraries(worker_proc MPI::MPI_C)