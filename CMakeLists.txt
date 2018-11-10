cmake_minimum_required(VERSION 2.8.3)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child} AND EXISTS ${curdir}/${child}/CMakeLists.txt)
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

#############################################################

set(project_name NEMATOD)
project(${project_name})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Werror=return-type -Og -g")

include_directories(.)

SUBDIRLIST(LIBS ${CMAKE_SOURCE_DIR})
list(REMOVE_ITEM LIBS tests)
list(REMOVE_ITEM LIBS build)
list(REMOVE_ITEM LIBS bin)
list(REMOVE_ITEM LIBS .git)
list(REMOVE_ITEM LIBS interface)
include_directories(${CMAKE_SOURCE_DIR})

FOREACH(lib ${LIBS})
       ADD_SUBDIRECTORY(${lib})
       INCLUDE_DIRECTORIES(${lib}/include)
ENDFOREACH()

ADD_SUBDIRECTORY(tests)
