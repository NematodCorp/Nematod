cmake_minimum_required(VERSION 2.8.3)

include_directories("include")

file(GLOB_RECURSE source_files "src/*.cpp")
file(GLOB_RECURSE header_files "include/*.hpp" "include/*.def" "src/*.hpp")

add_library(input STATIC ${header_files} ${source_files})
