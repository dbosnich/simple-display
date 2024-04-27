##--------------------------------------------------------------
## Copyright (c) David Bosnich <david.bosnich.public@gmail.com>
##
## This code is licensed under the MIT License, a copy of which
## can be found in the license.txt file included at the root of
## this distribution, or at https://opensource.org/licenses/MIT
##--------------------------------------------------------------

# Set the include and source directories.
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/source")

# Gather all source files.
file(GLOB_RECURSE source_files ${SOURCE_DIR}/*.h
                               ${SOURCE_DIR}/*.cpp
                               ${SOURCE_DIR}/*.m
                               ${SOURCE_DIR}/*.mm)

# Group all files for the IDE.
source_group(TREE "${INCLUDE_DIR}"
             PREFIX "include"
             FILES ${header_files})
source_group(TREE "${SOURCE_DIR}"
             PREFIX "source"
             FILES ${source_files})

# Determine the target platform.
set(PLATFORM_SUFFIX_Windows win32)
set(PLATFORM_SUFFIX_Darwin macos)
set(PLATFORM_SUFFIX_Linux linux)
set(TARGET_PLATFORM_SUFFIX ${PLATFORM_SUFFIX_${CMAKE_SYSTEM_NAME}})

# Filter platform agnostic source files.
set(source_files_platform_agnostic ${source_files})
list(FILTER source_files_platform_agnostic
     EXCLUDE REGEX "/*/platform/*")

# Filter platform specific source files.
set(source_files_platform_specific ${source_files})
list(FILTER source_files_platform_specific
     INCLUDE REGEX "/*/platform/*")

# Filter target platform source files.
set(source_files_platform_target ${source_files_platform_specific})
list(FILTER source_files_platform_target
     INCLUDE REGEX "/*/${TARGET_PLATFORM_SUFFIX}/*")

# Filter other platform specific files.
set(source_files_platform_other ${source_files_platform_specific})
list(FILTER source_files_platform_other
     EXCLUDE REGEX "/*/${TARGET_PLATFORM_SUFFIX}/*")

# Define the library (static).
add_library(${LIB_TARGET} STATIC ${source_files_platform_agnostic}
                                 ${source_files_platform_target})
target_sources(${LIB_TARGET} PUBLIC ${header_files})
target_compile_features(${LIB_TARGET} PUBLIC cxx_std_14)
target_include_directories(${LIB_TARGET} PUBLIC ${INCLUDE_DIR})
target_include_directories(${LIB_TARGET} PRIVATE ${SOURCE_DIR})
target_compile_options(${LIB_TARGET} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /GR->
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror -fno-rtti>)

# Ensure other platform specific files are displayed but not built.
target_sources(${LIB_TARGET} PRIVATE ${source_files_platform_other})
set_source_files_properties(${source_files_platform_other}
                            PROPERTIES HEADER_FILE_ONLY TRUE)

# Add OpenGL dependencies.
find_package(OpenGL)
if (${OpenGL_FOUND})
    target_link_libraries(${LIB_TARGET} OpenGL::GL)
    target_compile_definitions(${LIB_TARGET} PRIVATE OPENGL_SUPPORTED)
endif()

# Add Vulkan dependencies.
find_package(Vulkan OPTIONAL_COMPONENTS shaderc_combined MoltenVK)
if (${Vulkan_FOUND})
    target_compile_definitions(${LIB_TARGET} PRIVATE VULKAN_SUPPORTED)
    target_link_libraries(${LIB_TARGET} Vulkan::Vulkan Vulkan::shaderc_combined)
endif()

# Add platform specific dependencies.
if (${TARGET_PLATFORM_SUFFIX} STREQUAL macos)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(METAL_LIBRARY Metal REQUIRED)
    find_library(METAL_KIT_LIBRARY MetalKit REQUIRED)
    target_link_libraries(${LIB_TARGET} ${COCOA_LIBRARY}
                                        ${METAL_LIBRARY}
                                        ${METAL_KIT_LIBRARY})
    target_compile_options(${LIB_TARGET} PRIVATE -Wno-deprecated-declarations)
    if (${Vulkan_FOUND})
        target_compile_definitions(${LIB_TARGET} PRIVATE VK_USE_PLATFORM_METAL_EXT)
    endif()
elseif (${TARGET_PLATFORM_SUFFIX} STREQUAL linux)
    find_library(X11_LIBRARY X11 REQUIRED)
    target_link_libraries(${LIB_TARGET} ${X11_LIBRARY})
elseif (${TARGET_PLATFORM_SUFFIX} STREQUAL win32)
    target_link_libraries(${LIB_TARGET} d3d12.lib d3dcompiler.lib dxgi.lib)
    if (${Vulkan_FOUND})
        target_link_options(${LIB_TARGET} PUBLIC "/ignore:4099")
        target_compile_definitions(${LIB_TARGET} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
        #target_compile_definitions(${LIB_TARGET} PUBLIC DEFAULT_GRAPHICS_API=GraphicsAPI::VULKAN)
    endif()
endif()
