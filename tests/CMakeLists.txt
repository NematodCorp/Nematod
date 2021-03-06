file(GLOB test_files "*.cpp" "*.hpp")
file(GLOB utils_files "utils/*.cpp" "utils/*.hpp")
file(GLOB_RECURSE test_files_cpu "cpu/*.cpp" "cpu/*.hpp")
file(GLOB_RECURSE test_files_ppu "ppu/*.cpp" "ppu/*.hpp")
file(GLOB_RECURSE test_files_mappers "mappers/*.cpp" "mappers/*.hpp")

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

file(COPY "cpu/roms" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(COPY "ppu/roms" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(COPY "mappers/roms" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_executable(tests ${test_files} ${utils_files})
add_executable(tests_cpu ${test_files_cpu} ${utils_files})
add_executable(tests_ppu ${test_files_ppu} ${utils_files})
add_executable(tests_mappers ${test_files_mappers} ${utils_files})
target_link_libraries(tests gtest_main gtest rt pthread core nesloader libaco memory interrupts clock)
target_link_libraries(tests_cpu gtest_main gtest rt pthread cpu nesloader memory)
target_link_libraries(tests_ppu gtest_main gtest rt pthread input core nesloader sfml-graphics sfml-window sfml-system)
target_link_libraries(tests_mappers gtest_main gtest rt pthread input core nesloader)

add_test(unit_tests tests.out)
