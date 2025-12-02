# Examples for Vulkan STD-Parallel library

# Basic usage example
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/basic_usage.cpp")
    add_executable(basic_usage_example basic_usage.cpp)
    target_link_libraries(basic_usage_example PRIVATE vulkan_stdpar)
    target_include_directories(basic_usage_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "Building basic_usage example")
endif()

# Device selection example
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/device_selection.cpp")
    add_executable(device_selection_example device_selection.cpp)
    target_link_libraries(device_selection_example PRIVATE vulkan_stdpar)
    target_include_directories(device_selection_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "Building device_selection example")
endif()

# Algorithms demo
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/algorithms_demo.cpp")
    add_executable(algorithms_demo algorithms_demo.cpp)
    target_link_libraries(algorithms_demo PRIVATE vulkan_stdpar)
    target_include_directories(algorithms_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "Building algorithms_demo example")
endif()

# Install all examples that were built
install(TARGETS
    RUNTIME DESTINATION bin/examples
    OPTIONAL
)