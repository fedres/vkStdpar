/**
 * @file vulkan_stdpar.hpp
 * @brief Single-header bundle for Vulkan STD-Parallel library
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This is the main header file for the Vulkan STD-Parallel library.
 * Include this file to get access to all GPU-accelerated standard
 * parallel algorithms and unified_vector container.
 * 
 * Usage:
 *   #include <vulkan_stdpar/vulkan_stdpar.hpp>
 *   
 *   vulkan_stdpar::unified_vector<int> data = {1, 2, 3, 4, 5};
 *   std::sort(vulkan_stdpar::vulkan_par, data.begin(), data.end());
 */

#ifndef VULKAN_STDPAR_VULKAN_STDPAR_HPP
#define VULKAN_STDPAR_VULKAN_STDPAR_HPP

// Core components
#include "core/versioning_engine.hpp"
#include "core/device_selection.hpp"
#include "core/profiling.hpp"
#include "core/exceptions.hpp"

// Containers
#include "containers/unified_vector.hpp"

// Iterators (included by unified_vector.hpp)

// Algorithms
#include "algorithms/parallel_invoker.hpp"

// Main namespace
namespace vulkan_stdpar {

// Global execution policy instance (defined in parallel_invoker.hpp)
// extern const vulkan_parallel_policy vulkan_par;


// Library version information
struct version_info {
    static constexpr int major = 1;
    static constexpr int minor = 0;
    static constexpr int patch = 0;
    
    static constexpr const char* version_string() {
        return "1.0.0";
    }
};

// Library initialization and cleanup
void initialize();
void shutdown();

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_VULKAN_STDPAR_HPP