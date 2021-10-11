cmake_minimum_required(VERSION 3.0.0)
project(fs VERSION 0.2)

# This file was written by Chiro2001

include(CTest)
enable_testing()

message(STATUS "${PROJECT_NAME} started building on ${CMAKE_SYSTEM_NAME}!")

# Packages

# Include
set(include_dirs
      "${PROJECT_SOURCE_DIR}/src")
message(STATUS "Include dirs: ${include_dirs}")
include_directories(${include_dirs})

# Src

file(GLOB_RECURSE source_files
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/Fs.c"
        "${PROJECT_SOURCE_DIR}/src/listFile.c"
        "${PROJECT_SOURCE_DIR}/src/utility.c")
message(STATUS "Source files: ${source_files}")

# Resources
# file(GLOB_RECURSE resources "${PROJECT_SOURCE_DIR}/resource/*")
# message(STATUS "Copying resources file(s): ${resources}")
# file(COPY ${resources} DESTINATION ${CMAKE_BINARY_DIR}/)

# Linking
# link_libraries(-lpthread -lm)

set(CMAKE_C_FLAGS "-Wall -g -ggdb")

add_executable(fs ${PROJECT_SOURCE_DIR}/programs/main.c ${source_files})
add_executable(fs_color ${PROJECT_SOURCE_DIR}/programs/main.c ${source_files})

target_compile_options(fs_color PUBLIC -DCOLORED)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
