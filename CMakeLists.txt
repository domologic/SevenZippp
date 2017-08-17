cmake_minimum_required(VERSION 3.5.1)

include(cmake/GlobalFunctions.cmake)

project(sevenzippp)

set (CMAKE_CXX_STANDARD 11)

# add includes
add_subdirectory(LZMA/CPP)

# add source files
add_subdirectory(SevenZip++)
