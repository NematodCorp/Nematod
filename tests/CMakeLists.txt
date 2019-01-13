file(GLOB test_files "*.cpp" "*.hpp")
file(GLOB_RECURSE test_files_cpu "cpu/*.cpp" "cpu/*.hpp")
file(GLOB_RECURSE test_files_ppu "ppu/*.cpp" "ppu/*.hpp")

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

file(COPY "cpu/roms" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(COPY "ppu/roms" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_executable(tests ${test_files})
add_executable(tests_cpu ${test_files_cpu})
add_executable(tests_ppu ${test_files_ppu})
target_link_libraries(tests gtest_main gtest rt pthread libaco memory interrupts clock)
target_link_libraries(tests_cpu gtest_main gtest rt pthread memory interrupts clock nesloader cpu)
target_link_libraries(tests_ppu gtest_main gtest rt pthread libaco memory interrupts clock nesloader cpu_cycle_co ppu input sfml-graphics sfml-window sfml-system)

add_test(unit_tests tests.out)
add_test(unit_tests tests_cpu.out)
add_test(unit_tests tests_ppu.out)
