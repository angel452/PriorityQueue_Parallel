project("PriorityQueue_Parallel")
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads)

include_directories($(CMAKE_CURRENT_SOURCE_DIR)/source)

add_executable(priority_queue_parallel source/main.cpp)
target_link_libraries(priority_queue_parallel Threads::Threads)