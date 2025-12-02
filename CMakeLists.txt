cmake_minimum_required(VERSION 3.16)
project(VulkanStdpar VERSION 1.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
endif()

# Find required packages
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)

# Try to find Sylkan SYCL implementation
pkg_check_modules(SYLKAN QUIET sylkan)

if(SYLKAN_FOUND)
    message(STATUS "Found Sylkan SYCL implementation")
    add_compile_definitions(VULKAN_STDPAR_USE_SYLKAN)
else()
    # Try alternative SYCL implementations
    find_package(SYCL QUIET)
    if(SYCL_FOUND)
        message(STATUS "Found SYCL implementation")
        add_compile_definitions(VULKAN_STDPAR_USE_SYCL)
    else()
        message(WARNING "No SYCL implementation found. GPU acceleration will be disabled.")
        add_compile_definitions(VULKAN_STDPAR_NO_GPU)
    endif()
endif()

# Header-only library target
add_library(vulkan_stdpar INTERFACE)

# Target include directories
target_include_directories(vulkan_stdpar INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Target compile definitions
target_compile_definitions(vulkan_stdpar INTERFACE
    VULKAN_STDPAR_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    VULKAN_STDPAR_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    VULKAN_STDPAR_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# Link libraries
target_link_libraries(vulkan_stdpar INTERFACE Vulkan::Vulkan)

if(SYLKAN_FOUND)
    target_include_directories(vulkan_stdpar INTERFACE ${SYLKAN_INCLUDE_DIRS})
    target_link_libraries(vulkan_stdpar INTERFACE ${SYLKAN_LIBRARIES})
    target_compile_options(vulkan_stdpar INTERFACE ${SYLKAN_CFLAGS_OTHER})
elseif(SYCL_FOUND)
    target_link_libraries(vulkan_stdpar INTERFACE SYCL::SYCL)
endif()

# Export target
install(TARGETS vulkan_stdpar
    EXPORT VulkanStdparTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Create and install export targets
install(EXPORT VulkanStdparTargets
    FILE VulkanStdparTargets.cmake
    NAMESPACE VulkanStdpar::
    DESTINATION lib/cmake/VulkanStdpar
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VulkanStdparConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/VulkanStdparConfig.cmake"
    INSTALL_DESTINATION lib/cmake/VulkanStdpar
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/VulkanStdparConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/VulkanStdparConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VulkanStdparConfigVersion.cmake"
    DESTINATION lib/cmake/VulkanStdpar
)

# Enable testing
enable_testing()

# Add subdirectories
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks")
    add_subdirectory(benchmarks)
endif()

# Documentation option
option(BUILD_DOCUMENTATION "Create and install documentation" OFF)
if(BUILD_DOCUMENTATION)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
        set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in
                     ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(docs ALL
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
        install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}/html
                DESTINATION share/doc/VulkanStdpar)
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Vulkan STD-Parallel Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
if(SYLKAN_FOUND)
    message(STATUS "  SYCL implementation: Sylkan")
elseif(SYCL_FOUND)
    message(STATUS "  SYCL implementation: Generic")
else()
    message(STATUS "  SYCL implementation: None")
endif()
message(STATUS "  Vulkan found: ${Vulkan_FOUND}")
message(STATUS "")