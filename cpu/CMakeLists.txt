cmake_minimum_required(VERSION 2.8.3)

include_directories("include/")
include_directories("../ppu/include")
include_directories("../memory/include")

file(GLOB_RECURSE source_files "src/*.cpp")
file(GLOB_RECURSE header_files "include/*.hpp" "include/*.def" "src/*.hpp")

add_library(cpu STATIC ${header_files} ${source_files})
target_compile_definitions(cpu PRIVATE CPU6502_FLAVOR=NES6502)

add_library(cpu_cycle_co STATIC ${header_files} ${source_files})
target_compile_definitions(cpu_cycle_co PRIVATE CPU6502_FLAVOR=NES6502)
target_compile_definitions(cpu_cycle_co PRIVATE CPU6502_CYCLE_ACTION=co_yield)
