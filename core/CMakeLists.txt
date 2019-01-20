cmake_minimum_required(VERSION 2.8.3)

include_directories("include")
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

file(GLOB_RECURSE source_files "src/*.cpp")
file(GLOB_RECURSE header_files "include/*.hpp" "include/*.def" "src/*.hpp")

add_library(core STATIC ${header_files} ${source_files})

target_link_libraries(core gtest_main gtest rt pthread libaco memory interrupts clock cpu_cycle_co ppu input)
