cmake_minimum_required(VERSION 2.8.7)
project(seashell C)

set(CMAKE_C_STANDARD 11) # Maybe change this...
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(embedfile embedfile/embedfile.c src/mystring.c)
target_include_directories(embedfile PUBLIC include)

add_custom_command(
  OUTPUT fake0.txt
  COMMAND cp -rf ../src ../src2
  COMMENT "Creating copies of source files before modification"
  VERBATIM
  )
add_custom_command(
  OUTPUT fake1.txt
  COMMAND embedfile ../test.txt ../src/tests.c EMBED_FILE_HERE ../src/tests.c
  DEPENDS fake0.txt embedfile
  COMMENT "Embedding files within the code"
  VERBATIM
  )
add_custom_target(
  EmbedFiles ALL
  DEPENDS fake0.txt fake1.txt
)

find_package(CURL)
if (${CURL_FOUND})
  add_definitions(-DCURL_FOUND)
endif()

add_library(linenoise linenoise/linenoise.c)

file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)
add_executable(seashell ${SRCS})
target_link_libraries(seashell linenoise ${CURL_LIBRARIES})
target_include_directories(seashell PUBLIC include linenoise/ ${CURL_INCLUDE_DIRS})

add_custom_command(
  OUTPUT fakefile.txt
  COMMAND pwd
  COMMAND rm -rf ../src
  COMMAND mv ../src2 ../src
  COMMENT "Restoring source files now that seashell has been built"
  VERBATIM
  )
add_custom_target(
  CleanUp ALL
  DEPENDS fakefile.txt seashell
  )
