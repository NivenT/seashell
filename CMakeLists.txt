cmake_minimum_required(VERSION 2.8.7)
project(seashell C)

set(CMAKE_C_STANDARD 11) # Maybe change this...
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(embedfile embedfile/embedfile.c src/mystring.c)
target_include_directories(embedfile PUBLIC include)

add_custom_command(
  OUTPUT ../src2
  COMMAND cp -r ../src ../src2
  COMMENT "Creating copies of source files before modification"
  VERBATIM
  )
add_custom_command(
  OUTPUT ../src/tests.c
  COMMAND embedfile ../test.txt ../src/tests.c EMBED_FILE_HERE ../src2/tests.c
  COMMENT "Embedding files within the code"
  VERBATIM
  )
add_custom_target(
  EmbedFiles ALL
  DEPENDS ../src2 ../src/tests.c
)

find_package(CURL)
if (${CURL_FOUND})
  add_definitions(-DCURL_FOUND)
endif()

add_library(linenoise linenoise/linenoise.c)

file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src2/*.c)
add_executable(seashell src2/main.c)
target_link_libraries(seashell linenoise ${CURL_LIBRARIES})
target_include_directories(seashell PUBLIC include linenoise/ ${CURL_INCLUDE_DIRS})

add_custom_command(
  OUTPUT fakefile.txt
  COMMAND mv -f ../src2/* ../src
  COMMAND rm -rf ../src2
  COMMENT "Restoring source files"
  VERBATIM
  )
add_custom_target(
  CleanUp ALL
  DEPENDS fakefile.txt
  )
