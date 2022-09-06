################################################################################
# MIT License

# Copyright (c) 2022 FortModGen

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
################################################################################

cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project(FortModGen VERSION 1.0.0 LANGUAGES C CXX Fortran)

set (FortModGen_VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 17)

#Use the compilers found in the path
find_program(CMAKE_C_COMPILER 
  NAMES $ENV{CC} gcc 
  PATHS ENV 
  PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER 
  NAMES $ENV{CXX} g++ 
  PATHS ENV 
  PATH NO_DEFAULT_PATH)
find_program(CMAKE_FORTRAN_COMPILER 
  NAMES $ENV{FC} gfortran 
  PATHS ENV 
  PATH NO_DEFAULT_PATH)

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_INSTALL_PREFIX STREQUAL "" OR CMAKE_INSTALL_PREFIX STREQUAL
  "/usr/local")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
elseif(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
endif()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

#### RPATH handling #####

set(CMAKE_MACOSX_RPATH 1)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")

include(CPM)

CPMFindPackage(
  NAME toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
  VERSION 3.7.1
  GIT_TAG v3.7.1
)

CPMFindPackage(
    NAME fmt
    GIT_TAG 8.1.1
    GITHUB_REPOSITORY fmtlib/fmt
)

add_subdirectory(app)
add_subdirectory(src)

if(FORTMODGEN_TEST_ENABLED)
  include(CTest)
  add_subdirectory(test)
endif()

include(cmake/Modules/FortModGen.cmake)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/FortModGenConfig.cmake.in FortModGenConfig.cmake
  INSTALL_DESTINATION 
    /this/is/ignored/for/some/reason/thanks/kitware
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/FortModGenConfig.cmake 
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/FortModGen.cmake 
  DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake)

write_basic_package_version_file(${CMAKE_INSTALL_PREFIX}/cmake/FortModGenConfig.cmake
  VERSION ${FortModGen_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(EXPORT FortModGen-targets
  FILE FortModGenTargets.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake
)
