#CMakeLists.txt
#Copyright (c) 2009, Ewen Cheslack-Postava
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright notice,
#      this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright notice,
#      this list of conditions and the following disclaimer in the documentation
#      and/or other materials provided with the distribution.
#    * Neither the name of brain nor the names of its contributors
#      may be used to endorse or promote products derived from this software
#      without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
#ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#cmake options

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Default CACHE STRING
      "Build types include: Default Debug Release None RelWithDebInfo Profile Coverage." FORCE
      )
ENDIF()
IF(NOT WIN32)
SET(BRAIN_DEFAULT_COMPILE_FLAGS "-Wall -Wno-unused-function -Wno-unused-variable -fvisibility=hidden")
ENDIF()
SET(CMAKE_CXX_FLAGS ${BRAIN_DEFAULT_COMPILE_FLAGS})

# Default build target
SET(CMAKE_CXX_FLAGS_DEFAULT "-O0 -g2 ${BRAIN_DEFAULT_COMPILE_FLAGS}" CACHE STRING
    "Built cxxflags with default settings."
    FORCE )
SET( CMAKE_C_FLAGS_DEFAULT "-g2 ${BRAIN_DEFAULT_COMPILE_FLAGS}" CACHE STRING
    "Built cflags with default settings."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_DEFAULT
    "" CACHE STRING
    "Linking binaries with default settings."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_DEFAULT
    "" CACHE STRING
    "Building shared libraries with default settings."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_DEFAULT
    CMAKE_C_FLAGS_DEFAULT
    CMAKE_EXE_LINKER_FLAGS_DEFAULT
    CMAKE_SHARED_LINKER_FLAGS_DEFAULT )

# Profile build target
SET(CMAKE_CXX_FLAGS_PROFILE "-g2 -O3 -pg ${BRAIN_DEFAULT_COMPILE_FLAGS}" CACHE STRING
    "Built cxxflags with default settings."
    FORCE )
SET( CMAKE_C_FLAGS_PROFILE "-g2 -O3 -pg ${BRAIN_DEFAULT_COMPILE_FLAGS}" CACHE STRING
    "Built cflags with default settings."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_PROFILE
    "-pg" CACHE STRING
    "Linking binaries with default settings."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_PROFILE
    "-pg" CACHE STRING
    "Building shared libraries with default settings."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_PROFILE
    CMAKE_C_FLAGS_PROFILE
    CMAKE_EXE_LINKER_FLAGS_PROFILE
    CMAKE_SHARED_LINKER_FLAGS_PROFILE )

# Coverage build target
SET(CMAKE_CXX_FLAGS_COVERAGE "-g2 -fprofile-arcs -ftest-coverage ${BRAIN_DEFAULT_COMPILE_FLAGS}" CACHE STRING
    "Built cxxflags with default settings."
    FORCE )
SET( CMAKE_C_FLAGS_COVERAGE "-g2 -fprofile-arcs -ftest-coverage ${BRAIN_DEFAULT_COMPILE_FLAGS}" CACHE STRING
    "Built cflags with default settings."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "-fprofile-arcs -ftest-coverage" CACHE STRING
    "Linking binaries with default settings."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "-fprofile-arcs -ftest-coverage" CACHE STRING
    "Building shared libraries with default settings."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE )

#project global flags

PROJECT(brain C CXX)

IF(NOT TOP_LEVEL)
  SET(TOP_LEVEL ${CMAKE_CURRENT_SOURCE_DIR}/..)
ENDIF()
GET_FILENAME_COMPONENT(TOP_LEVEL ${TOP_LEVEL} ABSOLUTE)

IF(NOT DEPENDENCIES_ROOT)
  SET(DEPENDENCIES_ROOT ${TOP_LEVEL}/..)
ENDIF()

#add-ins

IF(NOT CMAKE_MODULE_PATH)
  SET(CMAKE_MODULE_PATH ${TOP_LEVEL}/build)
ENDIF()
GET_FILENAME_COMPONENT(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ABSOLUTE)

MACRO(VERIFY_VERSION package_name major minor subminor)
  IF (${package_name}_MAJOR_VERSION LESS ${major})
    MESSAGE(FATAL_ERROR "${package_name} out of date: ${major}.${minor}.${subminor}")
  ELSEIF(${package_name}_MAJOR_VERSION EQUAL ${major})
    IF(${package_name}_MINOR_VERSION LESS ${minor})
       MESSAGE(FATAL_ERROR "${package_name} out of date: ${major}.${minor}.${subminor}")
    ELSEIF(${package_name}_MINOR_VERSION EQUAL ${major})
      IF(${package_name}_SUBMINOR_VERSION LESS ${subminor})
        MESSAGE(FATAL_ERROR "${package_name} out of date: ${major}.${minor}.${subminor}")
      ENDIF()
    ENDIF()
  ENDIF()
ENDMACRO(VERIFY_VERSION)


#libraries

# first we find/verify sirikata and installed-sirikata, which will
# provide a lot of useful tools
SET(SIRIKATA_CODE_ROOT)
IF(EXISTS ${DEPENDENCIES_ROOT}/sirikata)
  SET(SIRIKATA_CODE_ROOT ${DEPENDENCIES_ROOT}/sirikata)
ENDIF()

SET(SIRIKATA_INSTALL_ROOT)
IF(EXISTS ${DEPENDENCIES_ROOT}/installed-sirikata)
  SET(SIRIKATA_INSTALL_ROOT ${DEPENDENCIES_ROOT}/installed-sirikata)
ELSE()
IF(EXISTS ${DEPENDENCIES_ROOT}/)
  SET(SIRIKATA_INSTALL_ROOT ${DEPENDENCIES_ROOT})
ENDIF()
ENDIF()


#dependency: sirikata
FIND_PACKAGE(Sirikata)


# add sirikata's cmake modules
IF(Sirikata_CMAKE_MODULES)
  SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${Sirikata_CMAKE_MODULES})
ENDIF()

IF(EXISTS ${DEPENDENCIES_ROOT}/installed-protobufs)
SET(PROTOCOLBUFFERS_ROOT ${DEPENDENCIES_ROOT}/installed-protobufs)
ELSE()
SET(PROTOCOLBUFFERS_ROOT ${DEPENDENCIES_ROOT})
ENDIF()
#dependency: protocolbuffers
IF(NOT PROTOCOLBUFFERS_ROOT)
  IF(Sirikata_PROTOBUFS_ROOT)
    SET(PROTOCOLBUFFERS_ROOT ${Sirikata_PROTOBUFS_ROOT})
  ENDIF()
ENDIF()
FIND_PACKAGE(ProtocolBuffers)
IF(NOT PROTOCOLBUFFERS_FOUND)
  MESSAGE(FATAL_ERROR "Cannot find Protocol Buffers")
ENDIF()

SET(BOOST_ROOT ${DEPENDENCIES_ROOT})
#dependency: boost > 1.35
IF(NOT BOOST_ROOT)
  IF(Sirikata_BOOST_ROOT)
    SET(BOOST_ROOT ${Sirikata_BOOST_ROOT})
  ENDIF()
ENDIF()
SET(Boost_FIND_REQUIRED TRUE)
IF(WIN32)
  FIND_PACKAGE(Boost)# COMPONENTS system thread date_time program_options)
ELSE()
  FIND_PACKAGE(Boost)# COMPONENTS system)
  #STRING(REPLACE "boost_system" "boost_thread" Boost_THREAD_LIBRARY ${Boost_SYSTEM_LIBRARY})
  #STRING(REPLACE "boost_system" "boost_regex" Boost_REGEX_LIBRARY ${Boost_SYSTEM_LIBRARY})
  #STRING(REPLACE "boost_system" "boost_date_time" Boost_DATE_TIME_LIBRARY ${Boost_SYSTEM_LIBRARY})
  #STRING(REPLACE "boost_system" "boost_program_options" Boost_PROGRAM_OPTIONS_LIBRARY ${Boost_SYSTEM_LIBRARY})
ENDIF()
VERIFY_VERSION(Boost 1 37 0)


#source file location
SET(BRAIN_SOURCE_DIR ${TOP_LEVEL})

#cxx flags
INCLUDE_DIRECTORIES(BEFORE
        ${BRAIN_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${Sirikata_INCLUDE_DIR}
        ${PROTOCOLBUFFERS_INCLUDE_DIRS}
        )

#source files
SET(BRAIN_CORE_SOURCES
)

ADD_CUSTOM_TARGET(genome
                  DEPENDS ${BRAIN_SOURCE_DIR}/genome.pb.cc ${BRAIN_SOURCE_DIR}/genome.pb.h)
ADD_CUSTOM_COMMAND(OUTPUT ${BRAIN_SOURCE_DIR}/genome.pb.cc ${BRAIN_SOURCE_DIR}/genome.pb.h
                   COMMAND ${PROTOCOLBUFFERS_COMPILER} ${BRAIN_SOURCE_DIR}/genome.proto -I${BRAIN_SOURCE_DIR} --cpp_out=${BRAIN_SOURCE_DIR}
                   DEPENDS ${BRAIN_SOURCE_DIR}/genome.proto)


SET(BRAIN_SOURCES
  ${BRAIN_SOURCE_DIR}/main.cpp
  ${BRAIN_SOURCE_DIR}/genome.pb.cc
)



#link flags
LINK_DIRECTORIES(
        ${Boost_LIBRARY_DIRS}
)

#libs
#ADD_LIBRARY(cbr-core STATIC ${BRAIN_CORE_SOURCES} ${BRAIN_PROTOCOL_SOURCES})

#binaries
ADD_EXECUTABLE(brain ${BRAIN_SOURCES})
ADD_DEPENDENCIES(brain genome)
TARGET_LINK_LIBRARIES(brain
#  cbr-core
        ${PROTOCOLBUFFERS_LIBRARIES}
        )
IF(NOT WIN32)
SET_TARGET_PROPERTIES(brain PROPERTIES LINK_FLAGS -lpthread)
ENDIF()