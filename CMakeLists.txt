##--------------------------------------------------------------
## Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
##
## This code is licensed under the MIT License, a copy of which
## can be found in the license.txt file included at the root of
## this distribution, or at https://opensource.org/licenses/MIT
##--------------------------------------------------------------

# Specify minimum tested CMake version.
cmake_minimum_required(VERSION 3.25.0)

# Define project settings.
project(simple_display
        LANGUAGES CXX
        VERSION 1.0)

# Specify supported CUDA architectures.
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(CMAKE_CUDA_ARCHITECTURES all-major)
endif()

# Gather header files.
file(GLOB_RECURSE header_files include/*.h)

# Add the library source.
set(LIB_TARGET "${PROJECT_NAME}")
add_subdirectory("source")

# Customize the predefined targets folder name.
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "HelperTargets")

# Add documentation.
add_subdirectory("docs")

# Add tests.
enable_testing()
add_subdirectory("tests")
