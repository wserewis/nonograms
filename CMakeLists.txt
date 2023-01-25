cmake_minimum_required(VERSION 3.19)
project(nonograms)

set(CMAKE_CXX_STANDARD 14)
# Add MPI Package to Project
find_package(MPI REQUIRED)

add_executable(nonograms main.cpp)
# Add libraries for code completion and compiling
target_link_libraries(nonograms PUBLIC MPI::MPI_CXX)