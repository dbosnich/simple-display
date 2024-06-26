##--------------------------------------------------------------
## Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
##
## This code is licensed under the MIT License, a copy of which
## can be found in the license.txt file included at the root of
## this distribution, or at https://opensource.org/licenses/MIT
##--------------------------------------------------------------

# Early out if generating a sub project.
if (NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    return()
endif()

# Find Doxygen.
find_package(Doxygen)

# Early out if not found.
if (NOT ${Doxygen_FOUND})
    return()
endif()

# Gather all files that might have relevant documentation,
# excluding all implementation and platform specific files.
file(GLOB_RECURSE doc_files ${PROJECT_SOURCE_DIR}/source/*)
list(FILTER doc_files EXCLUDE REGEX "/*_implementation.h")
list(FILTER doc_files EXCLUDE REGEX "/*/graphics/*")
list(FILTER doc_files EXCLUDE REGEX "/*/platform/*")
list (APPEND doc_files ${header_files})

# Set variables that will be used to configure the Doxyfile with
# the desired set of input files and the custom output directory.
string(REPLACE ";" " " doc_file_strings "${doc_files}")
set(DOXYGEN_INPUT_FILES ${doc_file_strings})
set(DOXYGEN_README_FILE "${PROJECT_SOURCE_DIR}/readme.md")
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)

# Configure the Doxyfile by replacing variables enclosed by @@
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Create the Doxygen output directory.
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# Add custom command to generate the documentation.
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${doc_files} ${DOXYGEN_README_FILE}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating documentation")

# Add custom target to regenerate documentation if input changes.
set(DOC_TARGET "DOC_BUILD")
add_custom_target(${DOC_TARGET} ALL DEPENDS ${DOXYGEN_INDEX_FILE})
set_target_properties(${DOC_TARGET} PROPERTIES FOLDER "HelperTargets")
