cmake_minimum_required(VERSION 2.8.7)
project(seashell C)

set(CMAKE_C_STANDARD 11) # Maybe change this...
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(embedfile embedfile/embedfile.c src/mystring.c)
target_include_directories(embedfile PUBLIC include)

add_custom_command(
  OUTPUT tests.c
  COMMAND embedfile ../test.txt ../src/tests.c.in EMBED_FILE_HERE tests.c
  DEPENDS embedfile ${CMAKE_CURRENT_SOURCE_DIR}/src/tests.c.in
  COMMENT "Embedding files within the code"
  VERBATIM
  )

find_package(CURL)
if (${CURL_FOUND})
  add_definitions(-DCURL_FOUND)
endif()

add_library(linenoise linenoise/linenoise.c)

file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)
add_executable(seashell
  ${SRCS}
  ${CMAKE_CURRENT_BINARY_DIR}/tests.c
)
target_link_libraries(seashell linenoise ${CURL_LIBRARIES})
target_include_directories(seashell PUBLIC include linenoise/ ${CURL_INCLUDE_DIRS})
