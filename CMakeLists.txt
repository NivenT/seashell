cmake_minimum_required(VERSION 2.8.7)
project(seashell C)

set(CMAKE_C_STANDARD 11) # Maybe change this...
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)
add_executable(seashell ${SRCS})
target_include_directories(seashell PUBLIC include)
