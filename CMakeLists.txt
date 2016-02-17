cmake_minimum_required(VERSION 2.8)
project (datacenter)

#include_directories ("${PROJECT_SOURCE_DIR}/src")

#include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}")

add_subdirectory(src)
