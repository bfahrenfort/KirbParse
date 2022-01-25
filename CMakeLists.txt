cmake_minimum_required(VERSION 3.16)
project(KirbParse C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)
set(LIBRARY_OUTPUT_PATH ../lib)

set(LIB_FILES src/kirbparse.c)
set(TEST_FILES src/test.c)

add_library(KirbParse_Static STATIC ${LIB_FILES})
add_library(KirbParse_Dynamic SHARED ${LIB_FILES})
add_executable(KirbTest ${TEST_FILES})
target_link_libraries(KirbTest KirbParse_Static)
