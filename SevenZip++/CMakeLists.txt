# collect all sources in the current folder
collect_source_files(
  ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCE_SEVENZIPPP
)

#---In order to build the Simple_Linux example, use this:---
#collect_source_files(
#  ${CMAKE_CURRENT_SOURCE_DIR}/../Samples/Simple/Simple_Linux.cpp
#  SOURCE_SEVENZIPPP
#)

group_sources(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Sevenzippp SHARED ${SOURCE_SEVENZIPPP})

#---In order to build the Simple_Linux example, use this:---
#add_executable(sevenzippp ${SOURCE_SEVENZIPPP}
#${CMAKE_CURRENT_SOURCE_DIR}/../Samples/Simple/Simple_Linux.cpp)

target_include_directories(Sevenzippp
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Sevenzippp
  LZMA
)

install(TARGETS Sevenzippp DESTINATION lib)
