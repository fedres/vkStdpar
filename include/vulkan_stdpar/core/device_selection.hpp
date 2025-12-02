/**
 * @file device_selection.hpp
 * @brief GPU device enumeration, selection, and management
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file contains device management functionality including device enumeration,
 * selection criteria, and queue management for vendor-agnostic GPU access.
 */

#ifndef VULKAN_STDPAR_CORE_DEVICE_SELECTION_HPP
#define VULKAN_STDPAR_CORE_DEVICE_SELECTION_HPP

#include "exceptions.hpp"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <thread>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>
#endif

#ifdef VULKAN_STDPAR_USE_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace vulkan_stdpar {

// Forward declarations
struct device_info;
struct queue_family_info;
enum class operation_type;

/**
 * @brief Queue family information for device selection
 */
struct queue_family_info {
    uint32_t family_index;                     ///< Queue family index
    uint32_t queue_count;                      ///< Number of queues in family
    bool supports_compute;                     ///< Compute operations support
    bool supports_transfer;                    ///< Transfer operations support
    bool supports_graphics;                    ///< Graphics operations support
    bool supports_sparse_binding;              ///< Sparse memory binding
    uint32_t timestamp_valid_bits;            ///< Timestamp valid bits
    uint32_t min_image_transfer_granularity;  ///< Minimum image transfer granularity
    
    queue_family_info() 
        : family_index(0)
        , queue_count(0)
        , supports_compute(false)
        , supports_transfer(false)
        , supports_graphics(false)
        , supports_sparse_binding(false)
        , timestamp_valid_bits(0)
        , min_image_transfer_granularity(0)
    {}
};

/**
 * @brief Queue properties structure
 */
struct queue_properties {
    uint32_t queue_family_index;
    bool supports_compute;
    bool supports_transfer;
    bool supports_timeline_semaphores;
    size_t min_sub_group_size;
    size_t max_sub_group_size;
    size_t preferred_work_group_size_multiple;
    
    queue_properties()
        : queue_family_index(0)
        , supports_compute(false)
        , supports_transfer(false)
        , supports_timeline_semaphores(false)
        , min_sub_group_size(1)
        , max_sub_group_size(32)
        , preferred_work_group_size_multiple(32)
    {}
};

/**
 * @brief Device information structure
 */
struct device_info {
    // Basic identification
    std::string name;                          ///< Device name
    std::string vendor;                        ///< Vendor name
    std::string driver_version;                ///< Driver version string
    std::string api_version;                   ///< Vulkan API version
    
    // Hardware capabilities
    size_t memory_size;                        ///< Total device memory (bytes)
    size_t max_compute_units;                  ///< Number of compute units
    size_t max_work_group_size;                ///< Maximum work-group size
    size_t max_work_items_per_compute_unit;    ///< Max work items per CU
    uint32_t max_compute_work_group_count[3];  ///< Max compute work group count
    
    // Feature support
    bool supports_timeline_semaphores;         ///< Timeline semaphore support
    bool supports_pinned_memory;               ///< Host-coherent memory support
    bool supports_sub_groups;                  ///< Sub-group operations
    bool supports_fp16;                        ///< Half-precision floating point
    bool supports_fp64;                        ///< Double-precision floating point
    bool supports_int8;                        ///< 8-bit integer operations
    bool supports_int16;                       ///< 16-bit integer operations
    bool supports_int64;                       ///< 64-bit integer operations
    
    // Queue families
    std::vector<queue_family_info> queue_families; ///< Available queue families
    
    // Performance characteristics
    double peak_performance;                   ///< Theoretical peak GFLOPS
    double memory_bandwidth;                   ///< Memory bandwidth GB/s
    uint32_t clock_frequency;                  ///< Core clock frequency (MHz)
    
    device_info() 
        : memory_size(0)
        , max_compute_units(0)
        , max_work_group_size(0)
        , max_work_items_per_compute_unit(0)
        , supports_timeline_semaphores(false)
        , supports_pinned_memory(false)
        , supports_sub_groups(false)
        , supports_fp16(false)
        , supports_fp64(false)
        , supports_int8(false)
        , supports_int16(false)
        , supports_int64(false)
        , peak_performance(0.0)
        , memory_bandwidth(0.0)
        , clock_frequency(0)
    {
        max_compute_work_group_count[0] = 0;
        max_compute_work_group_count[1] = 0;
        max_compute_work_group_count[2] = 0;
    }
    
    /**
     * @brief Check if device is suitable for compute operations
     * @return True if device meets minimum requirements
     */
    bool is_suitable() const {
        return supports_compute() && 
               // has_compute_queue_family() && // Relaxed check for CPU fallback
               memory_size > 0;
    }
    
    /**
     * @brief Calculate performance score for device selection
     * @return Performance score (higher is better)
     */
    double performance_score() const {
        double score = 0.0;
        
        // Memory size (30% weight)
        score += (memory_size / (1024.0 * 1024.0 * 1024.0)) * 0.3;
        
        // Compute units (25% weight)
        score += max_compute_units * 0.25;
        
        // Peak performance (25% weight)
        score += peak_performance * 0.01;
        
        // Memory bandwidth (20% weight)
        score += memory_bandwidth * 0.02;
        
        return score;
    }
    
    /**
     * @brief Get device summary string
     * @return Formatted device summary
     */
    std::string get_summary() const {
        return name + " (" + vendor + ", " + 
               std::to_string(memory_size / (1024*1024*1024)) + "GB)";
    }
    
private:
    bool supports_compute() const {
        if (queue_families.empty()) return true; // Assume true if no queue info (CPU fallback)
        for (const auto& family : queue_families) {
            if (family.supports_compute) return true;
        }
        return false;
    }
    
    bool has_compute_queue_family() const {
        if (queue_families.empty()) return true; // Assume true if no queue info
        for (const auto& family : queue_families) {
            if (family.supports_compute && family.queue_count > 0) return true;
        }
        return false;
    }
};

/**
 * @brief Operation type for queue selection
 */
enum class operation_type {
    compute,
    transfer,
    mixed
};

/**
 * @brief Device management namespace
 */
namespace device {

#ifdef VULKAN_STDPAR_USE_SYCL

/**
 * @brief Convert SYCL device to device_info
 */
inline device_info sycl_device_to_info(const sycl::device& dev) {
    device_info info;
    
    info.name = dev.get_info<sycl::info::device::name>();
    info.vendor = dev.get_info<sycl::info::device::vendor>();
    
    // Get memory size
    info.memory_size = dev.get_info<sycl::info::device::global_mem_size>();
    
    // Get compute units
    info.max_compute_units = dev.get_info<sycl::info::device::max_compute_units>();
    
    // Get work group size
    info.max_work_group_size = dev.get_info<sycl::info::device::max_work_group_size>();
    
    // Feature support
    info.supports_fp64 = dev.has(sycl::aspect::fp64);
    info.supports_fp16 = dev.has(sycl::aspect::fp16);
    
    return info;
}

inline std::vector<device_info> enumerate_devices() {
    std::vector<device_info> devices;
    
    try {
        auto platforms = sycl::platform::get_platforms();
        for (const auto& platform : platforms) {
            auto devs = platform.get_devices();
            for (const auto& dev : devs) {
                devices.push_back(sycl_device_to_info(dev));
            }
        }
    } catch (const sycl::exception& e) {
        // No devices available
    }
    
    return devices;
}

inline std::vector<device_info> enumerate_suitable_devices() {
    auto all_devices = enumerate_devices();
    std::vector<device_info> suitable;
    
    std::copy_if(all_devices.begin(), all_devices.end(),
                std::back_inserter(suitable),
                [](const device_info& dev) { return dev.is_suitable(); });
    
    return suitable;
}

inline device_info get_default_device() {
    try {
        sycl::device dev = sycl::device::get_devices(sycl::info::device_type::gpu)[0];
        return sycl_device_to_info(dev);
    } catch (...) {
        try {
            // Fallback to CPU
            sycl::device dev = sycl::device::get_devices(sycl::info::device_type::cpu)[0];
            return sycl_device_to_info(dev);
        } catch (...) {
            throw device_not_found_exception("No devices available");
        }
    }
}

#else // !VULKAN_STDPAR_USE_SYCL

// CPU-only fallback implementations

inline std::vector<device_info> enumerate_devices() {
    device_info cpu_device;
    cpu_device.name = "CPU (Fallback)";
    cpu_device.vendor = "Standard C++";
    cpu_device.memory_size = 1024ULL * 1024 * 1024 * 16;  // Assume 16GB
    cpu_device.max_compute_units = std::thread::hardware_concurrency();
    cpu_device.max_work_group_size = 1;
    
    return {cpu_device};
}

inline std::vector<device_info> enumerate_suitable_devices() {
    return enumerate_devices();
}

inline device_info get_default_device() {
    auto devices = enumerate_devices();
    if (devices.empty()) {
        throw device_not_found_exception("No devices available");
    }
    return devices[0];
}

#endif // VULKAN_STDPAR_USE_SYCL

inline device_info select_by_name(const std::string& device_name) {
    auto devices = enumerate_devices();
    auto it = std::find_if(devices.begin(), devices.end(),
                          [&device_name](const device_info& dev) {
                              return dev.name.find(device_name) != std::string::npos;
                          });
    
    if (it == devices.end()) {
        throw device_not_found_exception("Device not found: " + device_name);
    }
    
    return *it;
}

inline device_info select_by_vendor(const std::string& vendor_name) {
    auto devices = enumerate_devices();
    auto it = std::find_if(devices.begin(), devices.end(),
                          [&vendor_name](const device_info& dev) {
                              return dev.vendor.find(vendor_name) != std::string::npos;
                          });
    
    if (it == devices.end()) {
        throw device_not_found_exception("Vendor not found: " + vendor_name);
    }
    
    return *it;
}

inline device_info select_by_memory(size_t minimum_memory) {
    auto devices = enumerate_devices();
    auto it = std::find_if(devices.begin(), devices.end(),
                          [minimum_memory](const device_info& dev) {
                              return dev.memory_size >= minimum_memory;
                          });
    
    if (it == devices.end()) {
        throw device_not_found_exception("No device with sufficient memory");
    }
    
    return *it;
}

inline device_info select_by_performance(double min_performance_score) {
    auto devices = enumerate_devices();
    auto it = std::find_if(devices.begin(), devices.end(),
                          [min_performance_score](const device_info& dev) {
                              return dev.performance_score() >= min_performance_score;
                          });
    
    if (it == devices.end()) {
        throw device_not_found_exception("No device with sufficient performance");
    }
    
    return *it;
}

inline device_info select_optimal_device(operation_type, size_t) {
    // For now, just return default device
    return get_default_device();
}

inline std::vector<device_info> rank_devices_by_performance() {
    auto devices = enumerate_devices();
    std::sort(devices.begin(), devices.end(),
             [](const device_info& a, const device_info& b) {
                 return a.performance_score() > b.performance_score();
             });
    return devices;
}

inline std::vector<device_info> rank_devices_by_memory() {
    auto devices = enumerate_devices();
    std::sort(devices.begin(), devices.end(),
             [](const device_info& a, const device_info& b) {
                 return a.memory_size > b.memory_size;
             });
    return devices;
}

inline bool validate_device(const device_info& device) {
    return device.is_suitable();
}

inline bool is_device_available(const device_info& device) {
    auto devices = enumerate_devices();
    return std::any_of(devices.begin(), devices.end(),
                      [&device](const device_info& dev) {
                          return dev.name == device.name;
                      });
}

inline std::string get_device_status(const device_info& device) {
    if (!is_device_available(device)) {
        return "unavailable";
    }
    if (!validate_device(device)) {
        return "unsuitable";
    }
    return "ready";
}

} // namespace device

/**
 * @brief Queue management namespace
 */
namespace queue {

#ifdef VULKAN_STDPAR_USE_SYCL

static sycl::queue* g_default_queue = nullptr;

inline sycl::queue auto_select_queue() {
    try {
        return sycl::queue(sycl::gpu_selector_v);
    } catch (...) {
        return sycl::queue(sycl::cpu_selector_v);
    }
}

inline sycl::queue select_compute_queue() {
    return auto_select_queue();
}

inline sycl::queue select_transfer_queue() {
    return auto_select_queue();
}

inline sycl::queue select_optimal_queue(operation_type) {
    return auto_select_queue();
}

inline sycl::queue create_queue(const device_info&) {
    return auto_select_queue();
}

inline sycl::queue create_queue(const device_info&, uint32_t) {
    return auto_select_queue();
}

inline sycl::queue create_queue(const device_info&, const queue_family_info&) {
    return auto_select_queue();
}

inline void set_default_queue(const sycl::queue& queue) {
    if (g_default_queue) {
        delete g_default_queue;
    }
    g_default_queue = new sycl::queue(queue);
}

inline sycl::queue get_default_queue() {
    if (!g_default_queue) {
        g_default_queue = new sycl::queue(auto_select_queue());
    }
    return *g_default_queue;
}

inline void reset_default_queue() {
    if (g_default_queue) {
        delete g_default_queue;
        g_default_queue = nullptr;
    }
}

inline queue_properties get_queue_properties(const sycl::queue& queue) {
    queue_properties props;
    // TODO: Query actual properties
    props.supports_compute = true;
    props.supports_transfer = true;
    return props;
}

inline bool is_compute_queue(const sycl::queue& queue) {
    return true;
}

inline bool is_transfer_queue(const sycl::queue& queue) {
    return true;
}

inline bool supports_timeline_semaphores(const sycl::queue& queue) {
    return false;
}

#endif // VULKAN_STDPAR_USE_SYCL

} // namespace queue

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_DEVICE_SELECTION_HPP