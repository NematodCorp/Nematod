file(GLOB_RECURSE test_files "*.cpp" "*.hpp")

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

file(COPY "cpu/roms" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(COPY "ppu/roms" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_executable(tests ${test_files})
target_link_libraries(tests gtest_main gtest rt pthread libaco ${LIBS})

add_test(unit_tests tests.out)
