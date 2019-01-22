cmake_minimum_required(VERSION 2.8.3)

include_directories("include")
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

file(GLOB_RECURSE source_files "src/*.cpp")
file(GLOB_RECURSE header_files "include/*.hpp" "include/*.def" "src/*.hpp")

add_executable(nematod ${header_files} ${source_files})

target_link_libraries(nematod core nesloader sfml-graphics sfml-window sfml-system)
