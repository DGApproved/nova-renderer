cmake_minimum_required(VERSION 3.12)
project(nova-renderer)

option(NOVA_PACKAGE "Build only the library, nothing else." OFF)

############################################################
# Fix build type because of possible capitalization issues #
############################################################
if(NOT NOVA_PACKAGE AND CMAKE_BUILD_TYPE)
    string(TOLOWER ${CMAKE_BUILD_TYPE} TMP_BUILD_TYPE)
    set(BUILD_TYPE_FIXED_BY_NOVA FALSE)
    if("${TMP_BUILD_TYPE}" STREQUAL "debug" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(ORIGINAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Specifies the build type on single-configuration generators. (Fixed to Debug from '${TMP_BUILD_TYPE}' by nova-renderer)" FORCE)
        set(BUILD_TYPE_FIXED_BY_NOVA TRUE)
    elseif(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(ORIGINAL_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Specifies the build type on single-configuration generators. (Fixed to Release from '${TMP_BUILD_TYPE}' by nova-renderer)" FORCE)
        set(BUILD_TYPE_FIXED_BY_NOVA TRUE)
    endif()

    if(BUILD_TYPE_FIXED_BY_NOVA)
        message(WARNING "CMAKE_BUILD_TYPE build type has been fixed to ${CMAKE_BUILD_TYPE} from ${ORIGINAL_BUILD_TYPE}")
    endif()
elseif(NOT CMAKE_BUILD_TYPE AND NOT MSVC)
    set(CMAKE_BUILD_TYPE "Release")
endif()

#############################
# Options for nova's builds #
#############################
option(NOVA_TEST "Enable tests." OFF)
option(NOVA_ENABLE_EXPERIMENTAL "Enable experimental features, may be in code as well as in the CMake files" OFF)
option(NOVA_TREAT_WARNINGS_AS_ERRORS "Add -Werror flag or /WX for MSVC" OFF)
if(NOVA_ENABLE_EXPERIMENTAL)
    set(CMAKE_LINK_WHAT_YOU_USE TRUE) # Warn about unsued linked libraries
endif()

#########################################################
# Enable tests if not in submodule and debug or if MSVC #
#########################################################
if(NOT NOVA_PACKAGE)
    message(STATUS "Enabling nova tests by default because Nova is not a package")
    set(NOVA_TEST ON)
endif()

############################
# Initialize cmake options #
############################
set(3RD_PARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/external)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools/cmake" "${3RD_PARTY_DIR}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT NOVA_PACKAGE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

#################################
# Setup our own cmake additions #
#################################
include(ClangFormat)
include(ClangTidy)
include(RemovePermissive)
include(CompilerOptionsUtils)
include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)

set(BUILD_TESTING OFF CACHE BOOL "NO TESTING" FORCE)

#################################
# Setup 3rd party dependencies. #
#################################
include(external)

########################
# Add Nova source code #
########################
set(NOVA_SOURCE
        include/nova_renderer/util/platform.hpp
        src/loading/zip_folder_accessor.hpp
        src/loading/regular_folder_accessor.hpp
        src/loading/folder_accessor.hpp
        include/nova_renderer/nova_renderer.hpp
        src/nova_renderer.cpp
        src/loading/folder_accessor.cpp
        src/loading/regular_folder_accessor.cpp
        src/loading/zip_folder_accessor.cpp

        src/loading/shaderpack/shaderpack_loading.cpp
        src/loading/shaderpack/shaderpack_loading.hpp
        src/loading/loading_utils.cpp
        src/loading/loading_utils.hpp
        include/nova_renderer/shaderpack_data.hpp
        src/loading/shaderpack/shaderpack_data.cpp

        src/settings/nova_settings.cpp
        include/nova_renderer/nova_settings.hpp
        src/util/logger.cpp
        src/util/logger.hpp
        include/nova_renderer/render_engine.hpp
        include/nova_renderer/window.hpp
        src/loading/shaderpack/json_interop.cpp
        src/loading/shaderpack/json_interop.hpp
        src/loading/json_utils.hpp
        include/nova_renderer/util/utils.hpp
        src/util/utils.cpp
        include/nova_renderer/renderables.hpp
        src/render_objects/uniform_structs.hpp 
        src/loading/shaderpack/shaderpack_validator.cpp 
        src/loading/shaderpack/shaderpack_validator.hpp 
        src/loading/shaderpack/render_graph_builder.cpp 
        src/loading/shaderpack/render_graph_builder.hpp
        src/render_engine/vulkan/vulkan_render_engine.hpp
        src/render_engine/vulkan/vulkan_render_engine.cpp
        src/render_engine/vulkan/vulkan_render_engine_init.cpp
        src/render_engine/vulkan/vulkan_render_engine_shaderpack.cpp
        src/render_engine/vulkan/vulkan_render_engine_renderables.cpp
        src/render_engine/vulkan/vulkan_render_engine_render_frame.cpp
        src/render_engine/vulkan/vulkan_render_engine_mesh.cpp
        src/render_engine/vulkan/vulkan_utils.hpp
        src/render_engine/vulkan/vulkan_type_converters.hpp
        src/render_engine/vulkan/compacting_block_allocator.cpp 
        src/render_engine/vulkan/compacting_block_allocator.hpp 
        src/render_engine/vulkan/vulkan_utils.cpp 
        src/render_engine/vulkan/swapchain.cpp 
        src/render_engine/vulkan/swapchain.hpp
        src/render_engine/vulkan/auto_allocating_buffer.cpp
        src/render_engine/vulkan/auto_allocating_buffer.hpp
        src/render_engine/vulkan/cached_buffer.hpp
        src/render_engine/vulkan/cached_buffer.cpp
        src/render_engine/vulkan/struct_uniform_buffer.hpp
        src/render_engine/vulkan/struct_uniform_buffer.cpp
        src/render_engine/vulkan/fixed_size_buffer_allocator.hpp
        src/tasks/task_scheduler.cpp
        src/tasks/task_scheduler.hpp
        src/tasks/task_graph.hpp
        src/tasks/wait_free_queue.hpp
        src/tasks/condition_counter.cpp
        src/tasks/condition_counter.hpp

        src/debugging/renderdoc.cpp
        src/debugging/renderdoc.hpp
        include/nova_renderer/renderdoc_app.h

        include/nova_renderer/util/result.hpp
        src/util/result.cpp
        )

##################################
# Select platform dependent code #
##################################
set(WINDOWS_NOVA_SOURCE
    src/util/windows_utils.cpp
    src/util/windows_utils.hpp
    src/render_engine/dx12/pipeline_state_object.cpp
    src/render_engine/dx12/pipeline_state_object.hpp
    src/render_engine/dx12/vertex_attributes.hpp
    src/render_engine/dx12/dx12_texture.cpp
    src/render_engine/dx12/dx12_texture.hpp
    src/render_engine/dx12/dx12_utils.cpp
    src/render_engine/dx12/dx12_utils.hpp
    src/render_engine/dx12/win32_window.hpp
    src/render_engine/dx12/win32_window.cpp
    src/render_engine/dx12/dx12_render_engine.cpp
    src/render_engine/dx12/dx12_render_engine.hpp
    src/render_engine/dx12/d3dx12.h
)
set(UNIX_NOVA_SOURCE
    src/render_engine/vulkan/x11_window.cpp
    src/render_engine/vulkan/x11_window.hpp
    src/util/linux_utils.cpp
    src/util/linux_utils.hpp
)
if(WIN32)
    set(NOVA_SOURCE ${NOVA_SOURCE} ${WINDOWS_NOVA_SOURCE})
    set(OTHER_NOVA_SOURCE ${UNIX_NOVA_SOURCE})
else()
    set(NOVA_SOURCE ${NOVA_SOURCE} ${UNIX_NOVA_SOURCE})
    set(OTHER_NOVA_SOURCE ${WINDOWS_NOVA_SOURCE})
endif()

##########################################
# Set platform dependent compile options #
##########################################
if (UNIX)
    add_compile_options_if_supported(-g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=always)
endif()

if(MSVC)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS=1)
endif()

if(NOVA_TREAT_WARNINGS_AS_ERRORS)
    if(MSVC)
        add_compile_options(/WX)
    else()
        add_compile_options(-Werror)
    endif()
endif()

macro(add_coverage target)
    if(NOVA_COVERAGE)
        if(MSVC)
            message(FATAL_ERROR "Coverage not supported on MSVC.")
        else()
            target_compile_options(${target} PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
            target_link_libraries(${target} PRIVATE -g --coverage)
        endif()
    endif()
endmacro()

##########################################
# Setup required options for backtracing #
##########################################
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_options_if_supported(-g -rdynamic)
    message(STATUS "Enabling symbol output")
endif()

#################
# Organize code #
#################
# For now just put everthing in a single nova source group
# because there are several sub groups that would be extremely small
source_group("nova" FILES ${NOVA_NO_COMPILE} ${3RD_PARTY_SOURCE} ${NOVA_SOURCE})

##########################
# Setup the nova library #
##########################
add_library(nova-renderer STATIC
    ${NOVA_SOURCE}
    ${3RD_PARTY_SOURCE}
)
nova_format(nova-renderer "${OTHER_NOVA_SOURCE}")
target_include_directories(nova-renderer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
set_target_properties(nova-renderer PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_compile_definitions(nova-renderer PUBLIC DUMP_NOVA_CALLS)
target_compile_definitions(nova-renderer PUBLIC MTR_ENABLED)
target_compile_definitions(nova-renderer PUBLIC FMT_STRING_ALIAS=1)

include(GNUInstallDirs)

install(
    TARGETS nova-renderer
    EXPORT nova-renderer-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/
)

install(
    EXPORT nova-renderer-config DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/nova-renderer
    NAMESPACE nova-renderer::
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nova-renderer
    FILES_MATCHING PATTERN "*.hpp*"
)

##################################
# Add extra warnings if possible #
##################################
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options_if_supported(nova-renderer PUBLIC -Wall -Wextra -Wpedantic -Wconversion -Wdefaulted-function-deleted)
endif()

add_coverage(nova-renderer)

#########################
# Set libraries to link #
#########################
set(COMMON_LINK_LIBS ${CMAKE_DL_LIBS}
                     fmt::fmt
                     glm
                     glslang::glslang
                     glslang::OGLCompiler 
                     glslang::OSDependent
                     glslang::SPIRV
                     minitrace::minitrace
                     miniz::miniz
                     nlohmann_json
                     nlohmann_json::nlohmann_json
                     spirv-tools::SPIRV-Tools
                     spirv-cross-core
                     spirv-cross-cpp
                     spirv-cross-glsl
                     spirv-cross-reflect
                     spirv-cross-msl
                     spirv-cross-util
                     unofficial::vulkan-memory-allocator::vulkan-memory-allocator
                     Vulkan::Vulkan
)

##############################
# Find C++ filesystem header #
##############################
# On some toolchains its still <experimental/filesystem> while on others its already <filesystem>
check_include_file_cxx("filesystem" CXX_STD_FILESYSTEM)
if(NOT CXX_STD_FILESYSTEM)
    check_include_file_cxx("experimental/filesystem" CXX_STD_FILESYSTEM_EXPERIMENTAL)
    if(NOT CXX_STD_FILESYSTEM_EXPERIMENTAL)
        message(FATAL_ERROR "Failed to find a filesystem header for C++")
    endif()
    target_compile_definitions(nova-renderer PUBLIC NOVA_STD_FILESYSTEM_EXPERIMENTAL=1)
else()
    target_compile_definitions(nova-renderer PUBLIC NOVA_STD_FILESYSTEM=1)
endif()

############################
# Platform dependent fixes #
############################
if(WIN32)
    find_package(WindowsSDK)
    get_windowssdk_include_dirs(${WINDOWSSDK_LATEST_DIR} WinSDK_DIRS)

    # MSVC 15.9 broke the Windows SDK by implementing two phase lookup. "/Zc:twoPhase-" disables it.
    add_compile_options_if_supported(/Zc:twoPhase- /permissive /bigobj )

    set_target_properties(nova-renderer PROPERTIES PREFIX "")
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} d3d12 dxgi d3dcompiler glslang::HLSL spirv-cross-hlsl)
    target_include_directories(nova-renderer PRIVATE ${WinSDK_DIRS})
    target_compile_definitions(nova-renderer PUBLIC _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING NOMINMAX)

    # Visual Studio is stupid. Is it really that hard to use filesystem directories to organize code like literally
    # everyone else?
    foreach(source IN LISTS NOVA_SOURCE)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" source_path_msvc "${source_path}")
        source_group("${source_path_msvc}" FILES "${source}")
    endforeach()
    
else()
    # GCC and Clang complain loudly about the #pragma region stuff. This shuts them up.
    target_compile_options_if_supported(nova-renderer PRIVATE -Wno-unknown-pragmas)
    set(COMMON_LINK_LIBS ${COMMON_LINK_LIBS} stdc++fs X11)
endif()

###############################
# Link all required libraries #
###############################
target_link_libraries(nova-renderer PUBLIC ${COMMON_LINK_LIBS})

#############################
# Fix issue with permissive #
#############################
remove_permissive(nova-renderer)

##########################
# Add tests if requested #
##########################
if(NOVA_TEST)
    add_subdirectory(tests)
endif()
