/**
 * @file profiling.hpp
 * @brief Performance monitoring and profiling infrastructure
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file contains compile-time profiling infrastructure with thread-local
 * performance counters for monitoring GPU operations.
 */

#ifndef VULKAN_STDPAR_CORE_PROFILING_HPP
#define VULKAN_STDPAR_CORE_PROFILING_HPP

#include <cstdint>
#include <atomic>
#include <chrono>
#include <string>
#include <unordered_map>

namespace vulkan_stdpar {

/**
 * @brief Thread-local performance counters
 */
struct performance_counters {
    uint64_t bytes_copied_to_device;          ///< Host→device transfer bytes
    uint64_t bytes_copied_from_device;        ///< Device→host transfer bytes
    uint64_t kernel_launches;                  ///< Number of kernel executions
    double total_kernel_time;                  ///< Cumulative kernel execution time (seconds)
    double total_sync_time;                    ///< Cumulative synchronization time (seconds)
    uint64_t cache_hits;                       ///< Sync optimization hits
    uint64_t cache_misses;                     ///< Sync optimization misses
    
    /**
     * @brief Default constructor - initializes all counters to zero
     */
    performance_counters()
        : bytes_copied_to_device(0)
        , bytes_copied_from_device(0)
        , kernel_launches(0)
        , total_kernel_time(0.0)
        , total_sync_time(0.0)
        , cache_hits(0)
        , cache_misses(0)
    {}
    
    /**
     * @brief Reset all statistics to zero
     */
    void reset() {
        bytes_copied_to_device = 0;
        bytes_copied_from_device = 0;
        kernel_launches = 0;
        total_kernel_time = 0.0;
        total_sync_time = 0.0;
        cache_hits = 0;
        cache_misses = 0;
    }
    
    /**
     * @brief Get data throughput in GB/s
     * @return Throughput calculation
     */
    double get_throughput() const {
        if (total_kernel_time == 0.0) return 0.0;
        double total_bytes = static_cast<double>(bytes_copied_to_device + bytes_copied_from_device);
        return (total_bytes / (1024.0 * 1024.0 * 1024.0)) / total_kernel_time;
    }
    
    /**
     * @brief Get cache efficiency (hit rate)
     * @return Cache hit ratio (0.0 to 1.0)
     */
    double get_efficiency() const {
        uint64_t total = cache_hits + cache_misses;
        if (total == 0) return 0.0;
        return static_cast<double>(cache_hits) / static_cast<double>(total);
    }
    
    /**
     * @brief Get average kernel execution time
     * @return Average time in milliseconds
     */
    double get_avg_kernel_time() const {
        if (kernel_launches == 0) return 0.0;
        return (total_kernel_time * 1000.0) / static_cast<double>(kernel_launches);
    }
    
    /**
     * @brief Get total data transferred
     * @return Total bytes transferred
     */
    uint64_t get_total_transfer() const {
        return bytes_copied_to_device + bytes_copied_from_device;
    }
};

/**
 * @brief Profiling namespace for performance monitoring
 */
namespace profiling {

#ifdef VULKAN_STDPAR_ENABLE_PROFILING

/**
 * @brief Enable or disable profiling
 * @param enabled True to enable profiling
 */
void enable_profiling(bool enabled);

/**
 * @brief Check if profiling is enabled
 * @return True if profiling is currently enabled
 */
bool is_profiling_enabled();

/**
 * @brief Get thread-local performance counters
 * @return Reference to current thread's counters
 */
performance_counters& get_thread_counters();

/**
 * @brief Get performance metrics for specific queue
 * @param queue_id Queue identifier
 * @return Performance counters for queue
 */
performance_counters get_queue_metrics(uint32_t queue_id);

/**
 * @brief Get global aggregated metrics
 * @return Aggregated performance counters across all threads
 */
performance_counters get_global_metrics();

/**
 * @brief Reset thread-local counters
 */
void reset_thread_counters();

/**
 * @brief Reset all counters (thread-local and global)
 */
void reset_all_counters();

/**
 * @brief Record kernel launch
 * @param execution_time Kernel execution time in seconds
 */
void record_kernel_launch(double execution_time);

/**
 * @brief Record data transfer to device
 * @param bytes Number of bytes transferred
 * @param transfer_time Transfer time in seconds
 */
void record_transfer_to_device(uint64_t bytes, double transfer_time);

/**
 * @brief Record data transfer from device
 * @param bytes Number of bytes transferred
 * @param transfer_time Transfer time in seconds
 */
void record_transfer_from_device(uint64_t bytes, double transfer_time);

/**
 * @brief Record synchronization operation
 * @param sync_time Synchronization time in seconds
 * @param cache_hit True if sync was optimized (cache hit)
 */
void record_sync(double sync_time, bool cache_hit);

/**
 * @brief Print performance summary to stdout
 */
void print_summary();

/**
 * @brief Get performance summary as string
 * @return Formatted summary string
 */
std::string get_summary_string();

#else // VULKAN_STDPAR_ENABLE_PROFILING

// No-op implementations when profiling is disabled
inline void enable_profiling(bool) {}
inline bool is_profiling_enabled() { return false; }
inline performance_counters& get_thread_counters() { 
    static performance_counters dummy;
    return dummy;
}
inline performance_counters get_queue_metrics(uint32_t) { return performance_counters(); }
inline performance_counters get_global_metrics() { return performance_counters(); }
inline void reset_thread_counters() {}
inline void reset_all_counters() {}
inline void record_kernel_launch(double) {}
inline void record_transfer_to_device(uint64_t, double) {}
inline void record_transfer_from_device(uint64_t, double) {}
inline void record_sync(double, bool) {}
inline void print_summary() {}
inline std::string get_summary_string() { return ""; }

#endif // VULKAN_STDPAR_ENABLE_PROFILING

} // namespace profiling

/**
 * @brief RAII timer for automatic performance measurement
 */
class scoped_timer {
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;
    
    /**
     * @brief Construct timer and start timing
     * @param name Timer name for identification
     */
    explicit scoped_timer(const std::string& name = "")
        : name_(name)
        , start_(clock::now())
    {}
    
    /**
     * @brief Destructor - stops timer and reports elapsed time
     */
    ~scoped_timer() {
        auto end = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        elapsed_seconds_ = duration.count() / 1000000.0;
        
#ifdef VULKAN_STDPAR_ENABLE_PROFILING
        if (!name_.empty() && profiling::is_profiling_enabled()) {
            // Record timing if name suggests type
            if (name_.find("kernel") != std::string::npos) {
                profiling::record_kernel_launch(elapsed_seconds_);
            } else if (name_.find("sync") != std::string::npos) {
                profiling::record_sync(elapsed_seconds_, false);
            }
        }
#endif
    }
    
    /**
     * @brief Get elapsed time so far
     * @return Elapsed time in seconds
     */
    double elapsed() const {
        auto now = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_);
        return duration.count() / 1000000.0;
    }
    
    /**
     * @brief Get final elapsed time (after destruction)
     * @return Elapsed time in seconds
     */
    double get_elapsed_seconds() const {
        return elapsed_seconds_;
    }
    
private:
    std::string name_;
    time_point start_;
    double elapsed_seconds_ = 0.0;
};

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_PROFILING_HPP
