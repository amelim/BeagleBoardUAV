# CMakeLists for Beagle Bone Serial Logging Utility
# Sterling Peet
# Oct 23, 2013

cmake_minimum_required (VERSION 2.6)
project (bbLog)

# The version number.
set (bbLog_VERSION_MAJOR 0)
set (bbLog_VERSION_MINOR 1)

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

# Boost
set (Boost_USE_STATIC_LIBS   OFF)
set (Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS filesystem system thread)
include_directories(${Boost_INCLUDE_DIRS})

# Need pthreads
find_package (Threads)

MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
# MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARIES}")

# add the ASIOSerialPort library
include_directories ("${PROJECT_SOURCE_DIR}/serial")
include_directories ("${PROJECT_SOURCE_DIR}")
 
# add the main library
set(HEADER_FILES serial/ASIOSerialPort.h ${PROJECT_SOURCE_DIR}/events/Event.hpp ${PROJECT_SOURCE_DIR}/events/Delegate.hpp)

add_library(ASIOSerialPort serial/ASIOSerialPort.cpp ${HEADER_FILES})
 
install (TARGETS ASIOSerialPort DESTINATION bin)
install (FILES ${HEADER_FILES} DESTINATION include)

set (LIB_DEPS ${LIB_DEPS} ASIOSerialPort)

add_executable(bbLog bbLog.cpp ${HEADER_FILES})
target_link_libraries (bbLog ${LIB_DEPS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# add the install targets
install (TARGETS bbLog DESTINATION bin)
#install (FILES "${PROJECT_BINARY_DIR}/bbLog.h"        
#         DESTINATION include)