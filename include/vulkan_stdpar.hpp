/**
 * @file vulkan_stdpar.hpp
 * @brief Single-header bundle for Vulkan STD-Parallel library
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This is an automatically generated single-header version of the
 * Vulkan STD-Parallel library. Include this file to get access to
 * all GPU-accelerated standard parallel algorithms and unified_vector.
 * 
 * Usage:
 *   #include <vulkan_stdpar.hpp>
 *   
 *   vulkan_stdpar::unified_vector<int> data = {1, 2, 3, 4, 5};
 *   std::sort(data.begin(), data.end());
 */

#ifndef VULKAN_STDPAR_VULKAN_STDPAR_HPP
#define VULKAN_STDPAR_VULKAN_STDPAR_HPP

// Standard library includes
#include <atomic>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <functional>
#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <string>
#include <sstream>
#include <chrono>
#include <iterator>
#include <initializer_list>
#include <utility>
#include <limits>

// SYCL includes (optional)
#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>
#endif

// Vulkan includes (optional)
#ifdef VULKAN_STDPAR_USE_VULKAN
#include <vulkan/vulkan.h>
#endif


// ========== core/exceptions.hpp ==========

/**
 * @file exceptions.hpp
 * @brief Exception hierarchy for Vulkan STD-Parallel library
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file defines the exception hierarchy and error handling utilities
 * for the Vulkan STD-Parallel library.
 */


#include <stdexcept>
#include <string>
#include <sstream>

namespace vulkan_stdpar {

/**
 * @brief Base exception class for all Vulkan STD-Parallel exceptions
 */
class vulkan_stdpar_exception : public std::runtime_error {
public:
    explicit vulkan_stdpar_exception(const std::string& message)
        : std::runtime_error(message)
    {}
    
    explicit vulkan_stdpar_exception(const char* message)
        : std::runtime_error(message)
    {}
};

/**
 * @brief Exception thrown when host/device synchronization fails
 */
class synchronization_exception : public vulkan_stdpar_exception {
public:
    explicit synchronization_exception(const std::string& message)
        : vulkan_stdpar_exception("Synchronization error: " + message)
    {}
};

/**
 * @brief Exception thrown when kernel compilation fails
 */
class compilation_exception : public vulkan_stdpar_exception {
public:
    explicit compilation_exception(const std::string& message)
        : vulkan_stdpar_exception("Compilation error: " + message)
    {}
};

/**
 * @brief Exception thrown when GPU memory is exhausted
 */
class out_of_memory_exception : public vulkan_stdpar_exception {
public:
    explicit out_of_memory_exception(size_t requested_bytes)
        : vulkan_stdpar_exception(format_message(requested_bytes))
        , requested_bytes_(requested_bytes)
    {}
    
    size_t requested_bytes() const { return requested_bytes_; }
    
private:
    size_t requested_bytes_;
    
    static std::string format_message(size_t bytes) {
        std::ostringstream oss;
        oss << "Out of GPU memory: requested " 
            << (bytes / (1024.0 * 1024.0)) << " MB";
        return oss.str();
    }
};

/**
 * @brief Exception thrown when GPU device is lost or removed
 */
class device_lost_exception : public vulkan_stdpar_exception {
public:
    explicit device_lost_exception(const std::string& device_name = "")
        : vulkan_stdpar_exception("Device lost: " + 
            (device_name.empty() ? "unknown device" : device_name))
        , device_name_(device_name)
    {}
    
    const std::string& device_name() const { return device_name_; }
    
private:
    std::string device_name_;
};

/**
 * @brief Exception thrown when an unsupported operation is attempted
 */
class unsupported_operation_exception : public vulkan_stdpar_exception {
public:
    explicit unsupported_operation_exception(const std::string& operation)
        : vulkan_stdpar_exception("Unsupported operation: " + operation)
        , operation_(operation)
    {}
    
    const std::string& operation() const { return operation_; }
    
private:
    std::string operation_;
};

/**
 * @brief Exception thrown when device is not found
 */
class device_not_found_exception : public vulkan_stdpar_exception {
public:
    explicit device_not_found_exception(const std::string& criteria)
        : vulkan_stdpar_exception("Device not found: " + criteria)
        , criteria_(criteria)
    {}
    
    const std::string& criteria() const { return criteria_; }
    
private:
    std::string criteria_;
};

/**
 * @brief Exception thrown when device is unavailable
 */
class device_unavailable_exception : public vulkan_stdpar_exception {
public:
    explicit device_unavailable_exception(const std::string& reason)
        : vulkan_stdpar_exception("Device unavailable: " + reason)
        , reason_(reason)
    {}
    
    const std::string& reason() const { return reason_; }
    
private:
    std::string reason_;
};

/**
 * @brief Exception thrown when device initialization fails
 */
class device_initialization_exception : public vulkan_stdpar_exception {
public:
    explicit device_initialization_exception(const std::string& reason)
        : vulkan_stdpar_exception("Device initialization failed: " + reason)
        , reason_(reason)
    {}
    
    const std::string& reason() const { return reason_; }
    
private:
    std::string reason_;
};

/**
 * @brief Exception thrown when queue creation fails
 */
class queue_creation_exception : public vulkan_stdpar_exception {
public:
    explicit queue_creation_exception(const std::string& reason)
        : vulkan_stdpar_exception("Queue creation failed: " + reason)
        , reason_(reason)
    {}
    
    const std::string& reason() const { return reason_; }
    
private:
    std::string reason_;
};

/**
 * @brief Exception thrown when invalid argument is passed
 */
class invalid_argument_exception : public vulkan_stdpar_exception {
public:
    explicit invalid_argument_exception(const std::string& argument, const std::string& reason)
        : vulkan_stdpar_exception("Invalid argument '" + argument + "': " + reason)
        , argument_(argument)
        , reason_(reason)
    {}
    
    const std::string& argument() const { return argument_; }
    const std::string& reason() const { return reason_; }
    
private:
    std::string argument_;
    std::string reason_;
};

/**
 * @brief Error handling macro for try-catch blocks
 * 
 * Usage:
 *   VULKAN_STDPAR_TRY {
 *       // code that may throw
 *   }
 *   VULKAN_STD PAR_CATCH(error) {
 *       // handle error
 *   }
 */
#define VULKAN_STDPAR_TRY try

#define VULKAN_STDPAR_CATCH(exception_var) \
    catch (const vulkan_stdpar::vulkan_stdpar_exception& exception_var)

#define VULKAN_STDPAR_CATCH_ALL catch (...)

/**
 * @brief Assert macro that throws exception instead of aborting
 */
#ifdef VULKAN_STDPAR_DEBUG
#define VULKAN_STDPAR_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            throw vulkan_stdpar::vulkan_stdpar_exception( \
                std::string("Assertion failed: ") + #condition + " - " + message); \
        } \
    } while (0)
#else
#define VULKAN_STDPAR_ASSERT(condition, message) ((void)0)

/**
 * @brief Throw exception with formatted message
 */
#define VULKAN_STDPAR_THROW(exception_type, ...) \
    throw vulkan_stdpar::exception_type(__VA_ARGS__)

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_EXCEPTIONS_HPP

// ========== core/profiling.hpp ==========

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

// ========== core/versioning_engine.hpp ==========

/**
 * @file versioning_engine.hpp
 * @brief Memory state management and dirty tracking for unified data structures
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file contains the versioning_engine class that manages memory state
 * transitions and dirty range tracking for unified_vector containers.
 */


#include <atomic>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <algorithm>
#include <cassert>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>

namespace vulkan_stdpar {

/**
 * @brief Memory state enumeration for versioning engine
 */
enum class memory_state {
    clean,          ///< Host and device data synchronized
    host_dirty,     ///< Host has modifications, device needs update
    device_dirty     ///< Device has modifications, host needs update
};

/**
 * @brief Represents a contiguous region of modified memory with merge/overlap logic
 */
struct dirty_range {
    size_t start;        ///< Start index (inclusive)
    size_t end;          ///< End index (exclusive)
    
    /**
     * @brief Construct a dirty range
     * @param start Start index (inclusive)
     * @param end End index (exclusive)
     */
    dirty_range(size_t start = 0, size_t end = 0) : start(start), end(end) {
        assert(start <= end);
    }
    
    /**
     * @brief Check if this range overlaps with another
     * @param other Other range to check
     * @return True if ranges overlap
     */
    bool overlaps(const dirty_range& other) const {
        return start < other.end && other.start < end;
    }
    
    /**
     * @brief Check if this range is adjacent to another
     * @param other Other range to check
     * @return True if ranges are adjacent
     */
    bool adjacent(const dirty_range& other) const {
        return end == other.start || other.end == start;
    }
    
    /**
     * @brief Merge this range with another
     * @param other Other range to merge with
     * @return Merged range
     */
    dirty_range merge(const dirty_range& other) const {
        return dirty_range(std::min(start, other.start), std::max(end, other.end));
    }
    
    /**
     * @brief Get the size of this range
     * @return Range size in elements
     */
    size_t size() const {
        return end - start;
    }
    
    /**
     * @brief Check if this range is empty
     * @return True if range is empty
     */
    bool empty() const {
        return start >= end;
    }
    
    /**
     * @brief Check if this range contains an index
     * @param index Index to check
     * @return True if index is within range
     */
    bool contains(size_t index) const {
        return index >= start && index < end;
    }
};

/**
 * @brief Memory state manager with dirty range tracking
 * @tparam T Element type
 */
template<typename T>
class versioning_engine {
public:
    using value_type = T;
    using size_type = size_t;
    
private:
    mutable std::atomic<memory_state> state_;     ///< Current memory state
    mutable std::vector<dirty_range> dirty_ranges_; ///< Modified regions
    mutable std::shared_mutex mutex_;             ///< Thread safety
    
#ifdef VULKAN_STDPAR_USE_SYCL
    sycl::buffer<T> device_buffer_;            ///< Device memory buffer
#endif
    std::vector<T> host_data_;                  ///< Host memory storage
    size_type capacity_;                         ///< Allocated capacity
    bool device_allocated_;                       ///< Device buffer allocation flag
    
public:
    /**
     * @brief Construct versioning engine
     * @param capacity Initial capacity
     */
    explicit versioning_engine(size_type capacity = 0)
        : state_(memory_state::clean)
        , capacity_(capacity)
        , device_allocated_(false)
    {
        host_data_.reserve(capacity_);
    }
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~versioning_engine() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        sync_to_host_impl(lock);
    }
    
    // Non-copyable but movable
    versioning_engine(const versioning_engine&) = delete;
    versioning_engine& operator=(const versioning_engine&) = delete;
    
    versioning_engine(versioning_engine&& other) noexcept
        : state_(other.state_.load())
        , dirty_ranges_(std::move(other.dirty_ranges_))
        , host_data_(std::move(other.host_data_))
#ifdef VULKAN_STDPAR_USE_SYCL
        , device_buffer_(std::move(other.device_buffer_))
#endif
        , capacity_(other.capacity_)
        , device_allocated_(other.device_allocated_)
    {
        other.capacity_ = 0;
        other.device_allocated_ = false;
        other.state_.store(memory_state::clean);
    }
    
    versioning_engine& operator=(versioning_engine&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock1(mutex_, std::defer_lock);
            std::unique_lock<std::shared_mutex> lock2(other.mutex_, std::defer_lock);
            std::lock(lock1, lock2);
            
            state_ = other.state_.load();
            dirty_ranges_ = std::move(other.dirty_ranges_);
            host_data_ = std::move(other.host_data_);
#ifdef VULKAN_STDPAR_USE_SYCL
            device_buffer_ = std::move(other.device_buffer_);
#endif
            capacity_ = other.capacity_;
            device_allocated_ = other.device_allocated_;
            
            other.capacity_ = 0;
            other.device_allocated_ = false;
            other.state_.store(memory_state::clean);
        }
        return *this;
    }
    
    /**
     * @brief Get current memory state
     * @return Current memory state
     */
    memory_state get_memory_state() const noexcept {
        return state_.load(std::memory_order_acquire);
    }
    
    /**
     * @brief Check if device memory is dirty
     * @return True if device has modifications
     */
    bool is_device_dirty() const noexcept {
        return get_memory_state() == memory_state::device_dirty;
    }
    
    /**
     * @brief Check if host memory is dirty
     * @return True if host has modifications
     */
    bool is_host_dirty() const noexcept {
        return get_memory_state() == memory_state::host_dirty;
    }
    
    /**
     * @brief Check if memory is clean
     * @return True if host and device are synchronized
     */
    bool is_clean() const noexcept {
        return get_memory_state() == memory_state::clean;
    }
    
    /**
     * @brief Mark memory as host dirty
     * @param start Start index of dirty region
     * @param end End index of dirty region
     */
    void mark_host_dirty(size_type start, size_type end) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        mark_host_dirty_impl(lock, start, end);
    }
    
    /**
     * @brief Mark memory as device dirty
     */
    void mark_device_dirty() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        mark_device_dirty_impl(lock);
    }
    
    /**
     * @brief Synchronize host modifications to device
     */
    void sync_to_device() const {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        sync_to_device_impl(lock);
    }
    
    /**
     * @brief Synchronize device modifications to host
     */
    void sync_to_host() const {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        sync_to_host_impl(lock);
    }
    
    /**
     * @brief Resize storage capacity
     * @param new_capacity New capacity
     */
    void resize(size_type new_capacity) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        resize_impl(lock, new_capacity);
    }
    
    /**
     * @brief Get current capacity
     * @return Current capacity
     */
    size_type capacity() const noexcept {
        return capacity_;
    }
    
    /**
     * @brief Get host data pointer (read-only access)
     * @return Pointer to host data
     */
    const T* host_data() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return host_data_.data();
    }
    
    /**
     * @brief Get host data pointer (write access)
     * @return Pointer to host data
     */
    T* host_data() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return host_data_.data();
    }
    
#ifdef VULKAN_STDPAR_USE_SYCL
    /**
     * @brief Get device buffer
     * @return Reference to device buffer
     */
    sycl::buffer<T>& get_device_buffer() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        ensure_device_allocated(lock);
        return device_buffer_;
    }
    
    /**
     * @brief Get device buffer (const)
     * @return Const reference to device buffer
     */
    const sycl::buffer<T>& get_device_buffer() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        ensure_device_allocated(lock);
        return device_buffer_;
    }
#endif
    
    /**
     * @brief Clear all dirty ranges
     */
    void clear_dirty_ranges() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        dirty_ranges_.clear();
    }
    
    /**
     * @brief Get dirty ranges
     * @return Copy of dirty ranges
     */
    std::vector<dirty_range> get_dirty_ranges() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return dirty_ranges_;
    }
    
private:
    /**
     * @brief Implementation of mark_host_dirty with lock held
     */
    void mark_host_dirty_impl(std::unique_lock<std::shared_mutex>& lock, size_type start, size_type end) {
        // Validate range
        assert(start <= end);
        assert(end <= capacity_);
        
        if (start == end) return;
        
        dirty_range new_range(start, end);
        
        // Merge with existing ranges
        bool merged = false;
        for (auto& existing : dirty_ranges_) {
            if (existing.overlaps(new_range) || existing.adjacent(new_range)) {
                existing = existing.merge(new_range);
                merged = true;
                break;
            }
        }
        
        if (!merged) {
            dirty_ranges_.push_back(new_range);
        }
        
        // Merge overlapping ranges
        for (size_t i = 0; i < dirty_ranges_.size(); ++i) {
            for (size_t j = i + 1; j < dirty_ranges_.size(); ++j) {
                if (dirty_ranges_[i].overlaps(dirty_ranges_[j]) || 
                    dirty_ranges_[i].adjacent(dirty_ranges_[j])) {
                    dirty_ranges_[i] = dirty_ranges_[i].merge(dirty_ranges_[j]);
                    dirty_ranges_.erase(dirty_ranges_.begin() + j);
                    --j;
                }
            }
        }
        
        // Update state
        memory_state current = state_.load(std::memory_order_acquire);
        if (current == memory_state::clean) {
            state_.store(memory_state::host_dirty, std::memory_order_release);
        }
    }
    
    /**
     * @brief Implementation of mark_device_dirty with lock held
     */
    void mark_device_dirty_impl(std::unique_lock<std::shared_mutex>& lock) {
        dirty_ranges_.clear();
        state_.store(memory_state::device_dirty, std::memory_order_release);
    }
    
    /**
     * @brief Implementation of sync_to_device with lock held
     */
    void sync_to_device_impl(std::unique_lock<std::shared_mutex>& lock) const {
        if (get_memory_state() != memory_state::host_dirty) return;
        
#ifdef VULKAN_STDPAR_USE_SYCL
        ensure_device_allocated(lock);
        
        // Copy dirty ranges to device
        for (const auto& range : dirty_ranges_) {
            if (!range.empty()) {
                auto host_sub = host_data_.data() + range.start;
                auto device_sub = device_buffer_[sycl::range<1>(range.start)];
                auto size = range.size();
                
                // Submit copy command
                sycl::queue queue = get_default_queue();
                queue.submit([&](sycl::handler& cgh) {
                    auto device_acc = device_sub.get_access<sycl::access::mode::write>(cgh);
                    cgh.copy(host_sub, device_acc, size);
                });
            }
        }
        
        // Wait for completion
        get_default_queue().wait();
#endif
        
        dirty_ranges_.clear();
        state_.store(memory_state::clean, std::memory_order_release);
    }
    
    /**
     * @brief Implementation of sync_to_host with lock held
     */
    void sync_to_host_impl(std::unique_lock<std::shared_mutex>& lock) const {
        if (get_memory_state() != memory_state::device_dirty) return;
        
#ifdef VULKAN_STDPAR_USE_SYCL
        if (!device_allocated_) return;
        
        // Copy entire buffer from device to host
        sycl::queue queue = get_default_queue();
        queue.submit([&](sycl::handler& cgh) {
            auto device_acc = device_buffer_.get_access<sycl::access::mode::read>(cgh);
            cgh.copy(device_acc, host_data_.data());
        });
        
        queue.wait();
#endif
        
        state_.store(memory_state::clean, std::memory_order_release);
    }
    
    /**
     * @brief Implementation of resize with lock held
     */
    void resize_impl(std::unique_lock<std::shared_mutex>& lock, size_type new_capacity) {
        if (new_capacity <= capacity_) return;
        
        // Resize host storage
        host_data_.reserve(new_capacity);
        
#ifdef VULKAN_STDPAR_USE_SYCL
        if (device_allocated_) {
            // Create new device buffer
            sycl::buffer<T> new_buffer(new_capacity);
            
            // Copy old data if device is dirty
            if (get_memory_state() == memory_state::device_dirty) {
                sycl::queue queue = get_default_queue();
                queue.submit([&](sycl::handler& cgh) {
                    auto old_acc = device_buffer_.get_access<sycl::access::mode::read>(cgh);
                    auto new_acc = new_buffer.get_access<sycl::access::mode::write>(cgh);
                    cgh.copy(old_acc, new_acc, capacity_);
                });
                queue.wait();
            }
            
            device_buffer_ = std::move(new_buffer);
        }
#endif
        
        capacity_ = new_capacity;
    }
    
#ifdef VULKAN_STDPAR_USE_SYCL
    /**
     * @brief Ensure device buffer is allocated
     */
    void ensure_device_allocated(std::shared_lock<std::shared_mutex>& lock) const {
        if (!device_allocated_) {
            // Need to upgrade to unique lock
            lock.unlock();
            std::unique_lock<std::shared_mutex> unique_lock(mutex_);
            
            // Double-check after acquiring unique lock
            if (!device_allocated_) {
                device_buffer_ = sycl::buffer<T>(capacity_);
                device_allocated_ = true;
            }
            
            // Downgrade back to shared lock
            lock = std::shared_lock<std::shared_mutex>(std::move(unique_lock));
        }
    }
    
    /**
     * @brief Get default SYCL queue
     */
    static sycl::queue get_default_queue() {
        static sycl::queue default_queue;
        return default_queue;
    }
#endif
};

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_VERSIONING_ENGINE_HPP
// ========== core/device_selection.hpp ==========

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


#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <mutex>
#include <unordered_map>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>

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
               has_compute_queue_family() && 
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
        for (const auto& family : queue_families) {
            if (family.supports_compute) return true;
        }
        return false;
    }
    
    bool has_compute_queue_family() const {
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
 * @brief Device selection criteria
 */
struct device_selection_criteria {
    std::string preferred_vendor;
    size_t minimum_memory;
    double minimum_performance_score;
    bool require_timeline_semaphores;
    bool require_pinned_memory;
    bool prefer_integrated_gpu;
    
    device_selection_criteria()
        : minimum_memory(1024 * 1024 * 1024)  // 1GB minimum
        , minimum_performance_score(0.0)
        , require_timeline_semaphores(false)
        , require_pinned_memory(false)
        , prefer_integrated_gpu(false)
    {}
};

/**
 * @brief Queue selection criteria
 */
struct queue_selection_criteria {
    bool prefer_compute_queue;
    bool prefer_dedicated_queue;
    bool require_timeline_semaphores;
    size_t min_work_group_size;
    size_t max_work_group_size;
    
    queue_selection_criteria()
        : prefer_compute_queue(true)
        , prefer_dedicated_queue(false)
        , require_timeline_semaphores(false)
        , min_work_group_size(1)
        , max_work_group_size(1024)
    {}
};

/**
 * @brief Device management namespace
 */
namespace device {

/**
 * @brief Enumerate all available devices
 * @return Vector of device information
 */
std::vector<device_info> enumerate_devices();

/**
 * @brief Enumerate suitable devices for compute operations
 * @return Vector of suitable device information
 */
std::vector<device_info> enumerate_suitable_devices();

/**
 * @brief Get default device (highest performance)
 * @return Default device information
 */
device_info get_default_device();

/**
 * @brief Select device by name
 * @param device_name Device name to search for
 * @return Selected device information
 * @throws device_not_found_exception if device not found
 */
device_info select_by_name(const std::string& device_name);

/**
 * @brief Select device by vendor
 * @param vendor_name Vendor name to search for
 * @return Selected device information
 * @throws device_not_found_exception if device not found
 */
device_info select_by_vendor(const std::string& vendor_name);

/**
 * @brief Select device by minimum memory requirement
 * @param minimum_memory Minimum memory in bytes
 * @return Selected device information
 * @throws device_not_found_exception if no suitable device found
 */
device_info select_by_memory(size_t minimum_memory);

/**
 * @brief Select device by minimum performance score
 * @param min_performance_score Minimum performance score
 * @return Selected device information
 * @throws device_not_found_exception if no suitable device found
 */
device_info select_by_performance(double min_performance_score);

/**
 * @brief Select optimal device for given operation and data size
 * @param pattern Access pattern
 * @param data_size Data size in bytes
 * @return Selected device information
 */
device_info select_optimal_device(operation_type pattern, size_t data_size);

/**
 * @brief Rank devices by performance
 * @return Devices sorted by performance (best first)
 */
std::vector<device_info> rank_devices_by_performance();

/**
 * @brief Rank devices by memory size
 * @return Devices sorted by memory size (largest first)
 */
std::vector<device_info> rank_devices_by_memory();

/**
 * @brief Validate device availability and capabilities
 * @param device Device to validate
 * @return True if device is valid and available
 */
bool validate_device(const device_info& device);

/**
 * @brief Check if device is currently available
 * @param device Device to check
 * @return True if device is available
 */
bool is_device_available(const device_info& device);

/**
 * @brief Get device status string
 * @param device Device to query
 * @return Status description
 */
std::string get_device_status(const device_info& device);

} // namespace device

/**
 * @brief Queue management namespace
 */
namespace queue {

#ifdef VULKAN_STDPAR_USE_SYCL

/**
 * @brief Automatic queue selection
 * @return SYCL queue for optimal device
 */
sycl::queue auto_select_queue();

/**
 * @brief Select compute-optimized queue
 * @return SYCL queue for compute operations
 */
sycl::queue select_compute_queue();

/**
 * @brief Select transfer-optimized queue
 * @return SYCL queue for transfer operations
 */
sycl::queue select_transfer_queue();

/**
 * @brief Select queue for specific operation type
 * @param op Operation type
 * @return SYCL queue optimized for operation
 */
sycl::queue select_optimal_queue(operation_type op);

/**
 * @brief Create queue for specific device
 * @param device Device to create queue for
 * @return SYCL queue for device
 */
sycl::queue create_queue(const device_info& device);

/**
 * @brief Create queue for specific device and queue family
 * @param device Device to create queue for
 * @param queue_family_index Queue family index
 * @return SYCL queue for device and family
 */
sycl::queue create_queue(const device_info& device, uint32_t queue_family_index);

/**
 * @brief Create queue for specific device and queue family
 * @param device Device to create queue for
 * @param queue_family Queue family information
 * @return SYCL queue for device and family
 */
sycl::queue create_queue(const device_info& device, const queue_family_info& queue_family);

/**
 * @brief Get queue properties
 * @param queue SYCL queue
 * @return Queue properties
 */
queue_properties get_queue_properties(const sycl::queue& queue);

/**
 * @brief Check if queue supports compute operations
 * @param queue SYCL queue
 * @return True if queue supports compute
 */
bool is_compute_queue(const sycl::queue& queue);

/**
 * @brief Check if queue supports transfer operations
 * @param queue SYCL queue
 * @return True if queue supports transfer
 */
bool is_transfer_queue(const sycl::queue& queue);

/**
 * @brief Check if queue supports timeline semaphores
 * @param queue SYCL queue
 * @return True if queue supports timeline semaphores
 */
bool supports_timeline_semaphores(const sycl::queue& queue);

/**
 * @brief Set default queue for operations
 * @param queue SYCL queue to set as default
 */
void set_default_queue(const sycl::queue& queue);

/**
 * @brief Get default queue
 * @return Default SYCL queue
 */
sycl::queue get_default_queue();

/**
 * @brief Reset default queue to auto-selected
 */
void reset_default_queue();

#endif // VULKAN_STDPAR_USE_SYCL

} // namespace queue

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

// Exception declarations (to be defined in exceptions.hpp)
class device_not_found_exception;
class device_unavailable_exception;
class device_initialization_exception;
class queue_creation_exception;

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_DEVICE_SELECTION_HPP
// ========== containers/unified_reference.hpp ==========

/**
 * @file unified_reference.hpp
 * @brief Proxy reference type for write detection in unified_vector
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file implements the unified_reference proxy that enables write detection
 * for unified_vector elements while maintaining natural C++ syntax.
 */


#include <type_traits>
#include <utility>

namespace vulkan_stdpar {

// Forward declaration
template<typename T> class unified_vector;

/**
 * @brief Proxy reference for unified_vector elements
 * 
 * This class acts as a proxy reference that detects writes to vector elements
 * and triggers appropriate synchronization. It provides implicit conversion
 * for reads while marking the host as dirty on writes.
 * 
 * @tparam T Element type
 */
template<typename T>
class unified_reference {
private:
    unified_vector<T>* container_;     ///< Parent container
    size_t index_;                      ///< Element index
    
    // Friend declarations
    template<typename U> friend class unified_vector;
    template<typename U> friend class unified_iterator;
    
public:
    /**
     * @brief Construct reference to vector element
     * @param container Parent container
     * @param index Element index
     */
    unified_reference(unified_vector<T>* container, size_t index)
        : container_(container)
        , index_(index)
    {}
    
    /**
     * @brief Copy constructor
     */
    unified_reference(const unified_reference& other) = default;
    
    /**
     * @brief Implicit conversion to T for read access
     * @return Element value
     */
    operator T() const {
        return container_->at_impl(index_);
    }
    
    /**
     * @brief Assignment from value (triggers write detection)
     * @param value Value to assign
     * @return Reference to this
     */
    unified_reference& operator=(const T& value) {
        container_->set_impl(index_, value);
        return *this;
    }
    
    /**
     * @brief Assignment from another reference
     * @param other Other reference
     * @return Reference to this
     */
    unified_reference& operator=(const unified_reference& other) {
        if (this != &other) {
            container_->set_impl(index_, other.operator T());
        }
        return *this;
    }
    
    /**
     * @brief Move assignment
     * @param value Value to move
     * @return Reference to this
     */
    unified_reference& operator=(T&& value) {
        container_->set_impl(index_, std::move(value));
        return *this;
    }
    
    // Address operator disabled to prevent pointer escape
    T* operator&() = delete;
    
    // Compound assignment operators (trigger write detection)
    
    template<typename U>
    unified_reference& operator+=(const U& rhs) {
        T value = this->operator T();
        value += rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator-=(const U& rhs) {
        T value = this->operator T();
        value -= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator*=(const U& rhs) {
        T value = this->operator T();
        value *= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator/=(const U& rhs) {
        T value = this->operator T();
        value /= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator%=(const U& rhs) {
        T value = this->operator T();
        value %= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator&=(const U& rhs) {
        T value = this->operator T();
        value &= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator|=(const U& rhs) {
        T value = this->operator T();
        value |= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator^=(const U& rhs) {
        T value = this->operator T();
        value ^= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator<<=(const U& rhs) {
        T value = this->operator T();
        value <<= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    template<typename U>
    unified_reference& operator>>=(const U& rhs) {
        T value = this->operator T();
        value >>= rhs;
        container_->set_impl(index_, value);
        return *this;
    }
    
    // Increment/decrement operators
    
    unified_reference& operator++() {
        T value = this->operator T();
        ++value;
        container_->set_impl(index_, value);
        return *this;
    }
    
    T operator++(int) {
        T old_value = this->operator T();
        T new_value = old_value;
        ++new_value;
        container_->set_impl(index_, new_value);
        return old_value;
    }
    
    unified_reference& operator--() {
        T value = this->operator T();
        --value;
        container_->set_impl(index_, value);
        return *this;
    }
    
    T operator--(int) {
        T old_value = this->operator T();
        T new_value = old_value;
        --new_value;
        container_->set_impl(index_, new_value);
        return old_value;
    }
    
    /**
     * @brief Swap with another reference
     * @param other Other reference
     */
    void swap(unified_reference& other) {
        T temp = this->operator T();
        container_->set_impl(index_, other.operator T());
        other.container_->set_impl(other.index_, temp);
    }
    
    /**
     * @brief Get const pointer to element (read-only)
     * @return Const pointer
     */
    const T* get_ptr() const {
        return &container_->data_impl()[index_];
    }
};

/**
 * @brief Non-member swap for unified_reference
 * @tparam T Element type
 * @param lhs Left reference
 * @param rhs Right reference
 */
template<typename T>
void swap(unified_reference<T> lhs, unified_reference<T> rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace vulkan_stdpar


// ========== containers/unified_vector.hpp ==========

/**
 * @file unified_vector.hpp
 * @brief Drop-in replacement for std::vector with GPU acceleration
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file implements unified_vector, a container that provides 100% std::vector
 * API compatibility while enabling transparent GPU acceleration for parallel algorithms.
 */


#include <vector>
#include <initializer_list>
#include <algorithm>
#include <iterator>

namespace vulkan_stdpar {

// Forward declarations
template<typename T> class unified_iterator;
template<typename T> class const_unified_iterator;

/**
 * @brief Unified vector with automatic GPU acceleration
 * 
 * This class provides a drop-in replacement for std::vector that automatically
 * manages host/device memory synchronization and enables GPU acceleration for
 * standard parallel algorithms.
 * 
 * @tparam T Element type (must be trivially copyable)
 */
template<typename T>
class unified_vector {
public:
    // Type definitions (match std::vector)
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = unified_reference<T>;
    using const_reference = const T&;
    using pointer = void;  // Disabled to prevent pointer escape
    using const_pointer = const T*;
    using iterator = unified_iterator<T>;
    using const_iterator = const_unified_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
private:
    versioning_engine<T> engine_;     ///< Memory state management
    size_type size_;                   ///< Current element count
    
    // Friend declarations
    template<typename U> friend class unified_reference;
    template<typename U> friend class unified_iterator;
    template<typename U> friend class const_unified_iterator;
    
public:
    // ==================== Constructors ====================
    
    /**
     * @brief Default constructor
     */
    unified_vector() : engine_(0), size_(0) {}
    
    /**
     * @brief Construct with size
     * @param count Number of elements
     */
    explicit unified_vector(size_type count)
        : engine_(count), size_(count)
    {
        std::fill_n(data_impl(), count, T());
    }
    
    /**
     * @brief Construct with size and value
     * @param count Number of elements
     * @param value Initial value
     */
    unified_vector(size_type count, const T& value)
        : engine_(count), size_(count)
    {
        std::fill_n(data_impl(), count, value);
    }
    
    /**
     * @brief Construct from range
     * @tparam InputIt Iterator type
     * @param first Beginning of range
     * @param last End of range
     */
    template<typename InputIt>
    unified_vector(InputIt first, InputIt last)
        : engine_(std::distance(first, last))
        , size_(std::distance(first, last))
    {
        std::copy(first, last, data_impl());
    }
    
    /**
     * @brief Construct from initializer list
     * @param init Initializer list
     */
    unified_vector(std::initializer_list<T> init)
        : engine_(init.size())
        , size_(init.size())
    {
        std::copy(init.begin(), init.end(), data_impl());
    }
    
    /**
     * @brief Copy constructor
     * @param other Vector to copy
     */
    unified_vector(const unified_vector& other)
        : engine_(other.size_)
        , size_(other.size_)
    {
        // Sync other to host first
        other.engine_.sync_to_host();
        std::copy_n(other.data_impl(), size_, data_impl());
    }
    
    /**
     * @brief Move constructor
     * @param other Vector to move
     */
    unified_vector(unified_vector&& other) noexcept
        : engine_(std::move(other.engine_))
        , size_(other.size_)
    {
        other.size_ = 0;
    }
    
    /**
     * @brief Destructor
     */
    ~unified_vector() = default;
    
    // ==================== Assignment ====================
    
    /**
     * @brief Copy assignment
     * @param other Vector to copy
     * @return Reference to this
     */
    unified_vector& operator=(const unified_vector& other) {
        if (this != &other) {
            other.engine_.sync_to_host();
            engine_ = versioning_engine<T>(other.size_);
            size_ = other.size_;
            std::copy_n(other.data_impl(), size_, data_impl());
        }
        return *this;
    }
    
    /**
     * @brief Move assignment
     * @param other Vector to move
     * @return Reference to this
     */
    unified_vector& operator=(unified_vector&& other) noexcept {
        if (this != &other) {
            engine_ = std::move(other.engine_);
            size_ = other.size_;
            other.size_ = 0;
        }
        return *this;
    }
    
    /**
     * @brief Assign from initializer list
     * @param init Initializer list
     * @return Reference to this
     */
    unified_vector& operator=(std::initializer_list<T> init) {
        assign(init);
        return *this;
    }
    
    /**
     * @brief Assign from range
     * @tparam InputIt Iterator type
     * @param first Beginning of range
     * @param last End of range
     */
    template<typename InputIt>
    void assign(InputIt first, InputIt last) {
        size_type new_size = std::distance(first, last);
        if (new_size > capacity()) {
            engine_.resize(new_size);
        }
        size_ = new_size;
        std::copy(first, last, data_impl());
        engine_.mark_host_dirty(0, size_);
    }
    
    /**
     * @brief Assign from count copies of value
     * @param count Number of elements
     * @param value Value to copy
     */
    void assign(size_type count, const T& value) {
        if (count > capacity()) {
            engine_.resize(count);
        }
        size_ = count;
        std::fill_n(data_impl(), count, value);
        engine_.mark_host_dirty(0, size_);
    }
    
    /**
     * @brief Assign from initializer list
     * @param init Initializer list
     */
    void assign(std::initializer_list<T> init) {
        assign(init.begin(), init.end());
    }
    
    // ==================== Element Access ====================
    
    /**
     * @brief Access element with bounds checking
     * @param pos Element index
     * @return Reference to element
     * @throws std::out_of_range if pos >= size
*/
    reference at(size_type pos) {
        if (pos >= size_) {
            throw std::out_of_range("unified_vector::at: index out of range");
        }
        return reference(this, pos);
    }
    
    /**
     * @brief Access element with bounds checking (const)
     * @param pos Element index
     * @return Const reference to element
     * @throws std::out_of_range if pos >= size
     */
    const_reference at(size_type pos) const {
        if (pos >= size_) {
            throw std::out_of_range("unified_vector::at: index out of range");
        }
        return at_impl(pos);
    }
    
    /**
     * @brief Access element without bounds checking
     * @param pos Element index
     * @return Reference to element
     */
    reference operator[](size_type pos) {
        return reference(this, pos);
    }
    
    /**
     * @brief Access element without bounds checking (const)
     * @param pos Element index
     * @return Const reference to element
     */
    const_reference operator[](size_type pos) const {
        return at_impl(pos);
    }
    
    /**
     * @brief Access first element
     * @return Reference to first element
     */
    reference front() {
        return reference(this, 0);
    }
    
    /**
     * @brief Access first element (const)
     * @return Const reference to first element
     */
    const_reference front() const {
        return at_impl(0);
    }
    
    /**
     * @brief Access last element
     * @return Reference to last element
     */
    reference back() {
        return reference(this, size_ - 1);
    }
    
    /**
     * @brief Access last element (const)
     * @return Const reference to last element
     */
    const_reference back() const {
        return at_impl(size_ - 1);
    }
    
    /**
     * @brief Get pointer to underlying data (const)
     * @return Pointer to data
     */
    const T* data() const noexcept {
        engine_.sync_to_host();
        return data_impl();
    }
    
    // ==================== Iterators ====================
    
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
    
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;
    
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
    
    // ==================== Capacity ====================
    
    /**
     * @brief Check if vector is empty
     * @return True if size == 0
     */
    bool empty() const noexcept {
        return size_ == 0;
    }
    
    /**
     * @brief Get number of elements
     * @return Current size
     */
    size_type size() const noexcept {
        return size_;
    }
    
    /**
     * @brief Get maximum possible size
     * @return Maximum size
     */
    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }
    
    /**
     * @brief Reserve capacity
     * @param new_cap New capacity
     */
    void reserve(size_type new_cap) {
        if (new_cap > capacity()) {
            engine_.resize(new_cap);
        }
    }
    
    /**
     * @brief Get current capacity
     * @return Capacity
     */
    size_type capacity() const noexcept {
        return engine_.capacity();
    }
    
    /**
     * @brief Reduce capacity to fit size
     */
    void shrink_to_fit() {
        if (size_ < capacity()) {
            engine_.resize(size_);
        }
    }
    
    // ==================== Modifiers ====================
    
    /**
     * @brief Clear all elements
     */
    void clear() noexcept {
        size_ = 0;
        engine_.clear_dirty_ranges();
    }
    
    /**
     * @brief Insert element at position
     * @param pos Position to insert at
     * @param value Value to insert
     * @return Iterator to inserted element
     */
    iterator insert(const_iterator pos, const T& value);
    
    /**
     * @brief Insert element at position (move)
     * @param pos Position to insert at
     * @param value Value to insert
     * @return Iterator to inserted element
     */
    iterator insert(const_iterator pos, T&& value);
    
    /**
     * @brief Erase element at position
     * @param pos Position to erase
     * @return Iterator to next element
     */
    iterator erase(const_iterator pos);
    
    /**
     * @brief Erase range of elements
     * @param first Beginning of range
     * @param last End of range
     * @return Iterator to next element
     */
    iterator erase(const_iterator first, const_iterator last);
    
    /**
     * @brief Add element to end
     * @param value Value to add
     */
    void push_back(const T& value) {
        if (size_ >= capacity()) {
            size_type new_cap = capacity() == 0 ? 1 : capacity() * 2;
            reserve(new_cap);
        }
        data_impl()[size_] = value;
        engine_.mark_host_dirty(size_, size_ + 1);
        ++size_;
    }
    
    /**
     * @brief Add element to end (move)
     * @param value Value to add
     */
    void push_back(T&& value) {
        if (size_ >= capacity()) {
            size_type new_cap = capacity() == 0 ? 1 : capacity() * 2;
            reserve(new_cap);
        }
        data_impl()[size_] = std::move(value);
        engine_.mark_host_dirty(size_, size_ + 1);
        ++size_;
    }
    
    /**
     * @brief Construct element in place at end
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Reference to constructed element
     */
    template<typename... Args>
    reference emplace_back(Args&&... args) {
        if (size_ >= capacity()) {
            size_type new_cap = capacity() == 0 ? 1 : capacity() * 2;
            reserve(new_cap);
        }
        new (&data_impl()[size_]) T(std::forward<Args>(args)...);
        engine_.mark_host_dirty(size_, size_ + 1);
        return reference(this, size_++);
    }
    
    /**
     * @brief Remove last element
     */
    void pop_back() {
        if (size_ > 0) {
            --size_;
        }
    }
    
    /**
     * @brief Resize vector
     * @param count New size
     */
    void resize(size_type count) {
        resize(count, T());
    }
    
    /**
     * @brief Resize vector with value
     * @param count New size
     * @param value Value for new elements
     */
    void resize(size_type count, const T& value) {
        if (count > capacity()) {
            reserve(count);
        }
        if (count > size_) {
            std::fill_n(data_impl() + size_, count - size_, value);
            engine_.mark_host_dirty(size_, count);
        }
        size_ = count;
    }
    
    /**
     * @brief Swap with another vector
     * @param other Other vector
     */
    void swap(unified_vector& other) noexcept {
        std::swap(engine_, other.engine_);
        std::swap(size_, other.size_);
    }
    
    // ==================== Internal Implementation ====================
    
private:
    /**
     * @brief Get element value (implementation)
     */
    const T& at_impl(size_type index) const {
        engine_.sync_to_host();
        return data_impl()[index];
    }
    
    /**
     * @brief Set element value (implementation)
     */
    void set_impl(size_type index, const T& value) {
        engine_.sync_to_host();
        data_impl()[index] = value;
        engine_.mark_host_dirty(index, index + 1);
    }
    
    /**
     * @brief Set element value (implementation, move)
     */
    void set_impl(size_type index, T&& value) {
        engine_.sync_to_host();
        data_impl()[index] = std::move(value);
        engine_.mark_host_dirty(index, index + 1);
    }
    
    /**
     * @brief Get data pointer (mutable)
     */
    T* data_impl() {
        return engine_.host_data();
    }
    
    /**
     * @brief Get data pointer (const)
     */
    const T* data_impl() const {
        return engine_.host_data();
    }
    
public:
    // ==================== GPU Integration ====================
    
    /**
     * @brief Prefetch data to device
     */
    void prefetch_to_device() {
        engine_.sync_to_device();
    }
    
    /**
     * @brief Get versioning engine for GPU operations
     * @return Reference to versioning engine
     */
    versioning_engine<T>& get_engine() {
        return engine_;
    }
    
    /**
     * @brief Get versioning engine (const)
     * @return Const reference to versioning engine
     */
    const versioning_engine<T>& get_engine() const {
        return engine_;
    }
};

// ==================== Non-member Functions ====================

/**
 * @brief Equality comparison
 */
template<typename T>
bool operator==(const unified_vector<T>& lhs, const unified_vector<T>& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * @brief Inequality comparison
 */
template<typename T>
bool operator!=(const unified_vector<T>& lhs, const unified_vector<T>& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Less than comparison
 */
template<typename T>
bool operator<(const unified_vector<T>& lhs, const unified_vector<T>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/**
 * @brief Swap specialization
 */
template<typename T>
void swap(unified_vector<T>& lhs, unified_vector<T>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace vulkan_stdpar

// Include iterator implementation


// ========== iterators/unified_iterator.hpp ==========

/**
 * @file unified_iterator.hpp
 * @brief Iterator adaptors for unified_vector with sync-on-dereference
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file implements iterators for unified_vector that trigger synchronization
 * on dereference to maintain data consistency between host and device.
 */


#include <iterator>
#include <type_traits>

namespace vulkan_stdpar {

// Forward declarations
template<typename T> class unified_vector;
template<typename T> class unified_reference;

/**
 * @brief Mutable iterator for unified_vector
 * @tparam T Element type
 */
template<typename T>
class unified_iterator {
public:
    // Iterator traits
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = void;  // Disabled to prevent pointer escape
    using reference = unified_reference<T>;
    
private:
    unified_vector<T>* container_;    ///< Parent container
    size_t index_;                     ///< Current position
    
    // Friend declarations
    template<typename U> friend class unified_vector;
    template<typename U> friend class const_unified_iterator;
    
public:
    /**
     * @brief Default constructor
     */
    unified_iterator() : container_(nullptr), index_(0) {}
    
    /**
     * @brief Construct iterator
     * @param container Parent container
     * @param index Start position
     */
    unified_iterator(unified_vector<T>* container, size_t index)
        : container_(container), index_(index)
    {}
    
    /**
     * @brief Copy constructor
     */
    unified_iterator(const unified_iterator&) = default;
    
    /**
     * @brief Copy assignment
     */
    unified_iterator& operator=(const unified_iterator&) = default;
    
    /**
     * @brief Dereference operator
     * @return Reference to current element
     */
    reference operator*() const {
        return reference(container_, index_);
    }
    
    /**
     * @brief Subscript operator
     * @param n Offset
     * @return Reference to element at offset
     */
    reference operator[](difference_type n) const {
        return reference(container_, index_ + n);
    }
    
    // ==================== Increment/Decrement ====================
    
    /**
     * @brief Pre-increment
     */
    unified_iterator& operator++() {
        ++index_;
        return *this;
    }
    
    /**
     * @brief Post-increment
     */
    unified_iterator operator++(int) {
        unified_iterator temp = *this;
        ++index_;
        return temp;
    }
    
    /**
     * @brief Pre-decrement
     */
    unified_iterator& operator--() {
        --index_;
        return *this;
    }
    
    /**
     * @brief Post-decrement
     */
    unified_iterator operator--(int) {
        unified_iterator temp = *this;
        --index_;
        return temp;
    }
    
    // ==================== Arithmetic ====================
    
    /**
     * @brief Add offset
     */
    unified_iterator& operator+=(difference_type n) {
        index_ += n;
        return *this;
    }
    
    /**
     * @brief Subtract offset
     */
    unified_iterator& operator-=(difference_type n) {
        index_ -= n;
        return *this;
    }
    
    /**
     * @brief Add offset (non-modifying)
     */
    unified_iterator operator+(difference_type n) const {
        return unified_iterator(container_, index_ + n);
    }
    
    /**
     * @brief Subtract offset (non-modifying)
     */
    unified_iterator operator-(difference_type n) const {
        return unified_iterator(container_, index_ - n);
    }
    
    /**
     * @brief Distance between iterators
     */
    difference_type operator-(const unified_iterator& other) const {
        return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_);
    }
    
    // ==================== Comparison ====================
    
    /**
     * @brief Equality comparison
     */
    bool operator==(const unified_iterator& other) const {
        return container_ == other.container_ && index_ == other.index_;
    }
    
    /**
     * @brief Inequality comparison
     */
    bool operator!=(const unified_iterator& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief Less than comparison
     */
    bool operator<(const unified_iterator& other) const {
        return index_ < other.index_;
    }
    
    /**
     * @brief Less than or equal comparison
     */
    bool operator<=(const unified_iterator& other) const {
        return index_ <= other.index_;
    }
    
    /**
     * @brief Greater than comparison
     */
    bool operator>(const unified_iterator& other) const {
        return index_ > other.index_;
    }
    
    /**
     * @brief Greater than or equal comparison
     */
    bool operator>=(const unified_iterator& other) const {
        return index_ >= other.index_;
    }
    
    /**
     * @brief Get current index
     * @return Index into container
     */
    size_t get_index() const {
        return index_;
    }
    
    /**
     * @brief Get container pointer
     * @return Pointer to container
     */
    unified_vector<T>* get_container() const {
        return container_;
    }
};

/**
 * @brief Const iterator for unified_vector
 * @tparam T Element type
 */
template<typename T>
class const_unified_iterator {
public:
    // Iterator traits
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;
    
private:
    const unified_vector<T>* container_;    ///< Parent container
    size_t index_;                           ///< Current position
    
    // Friend declarations
    template<typename U> friend class unified_vector;
    
public:
    /**
     * @brief Default constructor
     */
    const_unified_iterator() : container_(nullptr), index_(0) {}
    
    /**
     * @brief Construct const iterator
     * @param container Parent container
     * @param index Start position
     */
    const_unified_iterator(const unified_vector<T>* container, size_t index)
        : container_(container), index_(index)
    {}
    
    /**
     * @brief Construct from mutable iterator
     * @param other Mutable iterator
     */
    const_unified_iterator(const unified_iterator<T>& other)
        : container_(other.get_container()), index_(other.get_index())
    {}
    
    /**
     * @brief Copy constructor
     */
    const_unified_iterator(const const_unified_iterator&) = default;
    
    /**
     * @brief Copy assignment
     */
    const_unified_iterator& operator=(const const_unified_iterator&) = default;
    
    /**
     * @brief Dereference operator
     * @return Const reference to current element
     */
    reference operator*() const {
        return (*container_)[index_];
    }
    
    /**
     * @brief Subscr operator
     * @param n Offset
     * @return Const reference to element at offset
     */
    reference operator[](difference_type n) const {
        return (*container_)[index_ + n];
    }
    
    // ==================== Increment/Decrement ====================
    
    const_unified_iterator& operator++() {
        ++index_;
        return *this;
    }
    
    const_unified_iterator operator++(int) {
        const_unified_iterator temp = *this;
        ++index_;
        return temp;
    }
    
    const_unified_iterator& operator--() {
        --index_;
        return *this;
    }
    
    const_unified_iterator operator--(int) {
        const_unified_iterator temp = *this;
        --index_;
        return temp;
    }
    
    // ==================== Arithmetic ====================
    
    const_unified_iterator& operator+=(difference_type n) {
        index_ += n;
        return *this;
    }
    
    const_unified_iterator& operator-=(difference_type n) {
        index_ -= n;
        return *this;
    }
    
    const_unified_iterator operator+(difference_type n) const {
        return const_unified_iterator(container_, index_ + n);
    }
    
    const_unified_iterator operator-(difference_type n) const {
        return const_unified_iterator(container_, index_ - n);
    }
    
    difference_type operator-(const const_unified_iterator& other) const {
        return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_);
    }
    
    // ==================== Comparison ====================
    
    bool operator==(const const_unified_iterator& other) const {
        return container_ == other.container_ && index_ == other.index_;
    }
    
    bool operator!=(const const_unified_iterator& other) const {
        return !(*this == other);
    }
    
    bool operator<(const const_unified_iterator& other) const {
        return index_ < other.index_;
    }
    
    bool operator<=(const const_unified_iterator& other) const {
        return index_ <= other.index_;
    }
    
    bool operator>(const const_unified_iterator& other) const {
        return index_ > other.index_;
    }
    
    bool operator>=(const const_unified_iterator& other) const {
        return index_ >= other.index_;
    }
    
    size_t get_index() const {
        return index_;
    }
    
    const unified_vector<T>* get_container() const {
        return container_;
    }
};

// ==================== Non-member Operators ====================

/**
 * @brief Add offset to iterator (reversed operands)
 */
template<typename T>
unified_iterator<T> operator+(typename unified_iterator<T>::difference_type n, 
                              const unified_iterator<T>& it) {
    return it + n;
}

/**
 * @brief Add offset to const iterator (reversed operands)
 */
template<typename T>
const_unified_iterator<T> operator+(typename const_unified_iterator<T>::difference_type n,
                                    const const_unified_iterator<T>& it) {
    return it + n;
}

} // namespace vulkan_stdpar

// Implementation of unified_vector iterator member functions
namespace vulkan_stdpar {

template<typename T>
typename unified_vector<T>::iterator unified_vector<T>::begin() noexcept {
    return iterator(this, 0);
}

template<typename T>
typename unified_vector<T>::const_iterator unified_vector<T>::begin() const noexcept {
    return const_iterator(this, 0);
}

template<typename T>
typename unified_vector<T>::const_iterator unified_vector<T>::cbegin() const noexcept {
    return const_iterator(this, 0);
}

template<typename T>
typename unified_vector<T>::iterator unified_vector<T>::end() noexcept {
    return iterator(this, size_);
}

template<typename T>
typename unified_vector<T>::const_iterator unified_vector<T>::end() const noexcept {
    return const_iterator(this, size_);
}

template<typename T>
typename unified_vector<T>::const_iterator unified_vector<T>::cend() const noexcept {
    return const_iterator(this, size_);
}

template<typename T>
typename unified_vector<T>::iterator 
unified_vector<T>::insert(const_iterator pos, const T& value) {
    size_type insert_pos = pos.get_index();
    if (size_ >= capacity()) {
        reserve(capacity() == 0 ? 1 : capacity() * 2);
    }
    
    // Shift elements right
    engine_.sync_to_host();
    std::copy_backward(data_impl() + insert_pos, data_impl() + size_, 
                      data_impl() + size_ + 1);
    data_impl()[insert_pos] = value;
    engine_.mark_host_dirty(insert_pos, size_ + 1);
    ++size_;
    
    return iterator(this, insert_pos);
}

template<typename T>
typename unified_vector<T>::iterator 
unified_vector<T>::insert(const_iterator pos, T&& value) {
    size_type insert_pos = pos.get_index();
    if (size_ >= capacity()) {
        reserve(capacity() == 0 ? 1 : capacity() * 2);
    }
    
    // Shift elements right
    engine_.sync_to_host();
    std::copy_backward(data_impl() + insert_pos, data_impl() + size_,
                      data_impl() + size_ + 1);
    data_impl()[insert_pos] = std::move(value);
    engine_.mark_host_dirty(insert_pos, size_ + 1);
    ++size_;
    
    return iterator(this, insert_pos);
}

template<typename T>
typename unified_vector<T>::iterator 
unified_vector<T>::erase(const_iterator pos) {
    size_type erase_pos = pos.get_index();
    engine_.sync_to_host();
    
    // Shift elements left
    std::copy(data_impl() + erase_pos + 1, data_impl() + size_,
             data_impl() + erase_pos);
    --size_;
    engine_.mark_host_dirty(erase_pos, size_);
    
    return iterator(this, erase_pos);
}

template<typename T>
typename unified_vector<T>::iterator 
unified_vector<T>::erase(const_iterator first, const_iterator last) {
    size_type first_pos = first.get_index();
    size_type last_pos = last.get_index();
    size_type count = last_pos - first_pos;
    
    if (count == 0) return iterator(this, first_pos);
    
    engine_.sync_to_host();
    
    // Shift elements left
    std::copy(data_impl() + last_pos, data_impl() + size_,
             data_impl() + first_pos);
    size_ -= count;
    engine_.mark_host_dirty(first_pos, size_);
    
    return iterator(this, first_pos);
}

} // namespace vulkan_stdpar


// ========== algorithms/parallel_invoker.hpp ==========

/**
 * @file parallel_invoker.hpp
 * @brief GPU algorithm execution engine for standard parallel algorithms
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @ date 2025-12-02
 * 
 * This file implements GPU execution of standard parallel algorithms using
 * SYCL backend. It provides ADL-based overloads and custom execution policy.
 */


#include <algorithm>
#include <numeric>
#include <functional>
#include <type_traits>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>

namespace vulkan_stdpar {

/**
 * @brief Execution policy for GPU parallel algorithms
 */
struct vulkan_parallel_policy {
#ifdef VULKAN_STDPAR_USE_SYCL
    sycl::queue* queue_ptr;  ///< SYCL execution queue
    
    /**
     * @brief Construct with automatic queue selection
     */
    vulkan_parallel_policy() : queue_ptr(nullptr) {}
    
    /**
     * @brief Construct with specific queue
     * @param q SYCL queue reference
     */
    explicit vulkan_parallel_policy(sycl::queue& q) : queue_ptr(&q) {}
    
    /**
     * @brief Get queue for execution
     * @return SYCL queue reference
     */
    sycl::queue& get_queue() const {
        if (queue_ptr) return *queue_ptr;
        return queue::get_default_queue();
    }
    
    /**
     * @brief Set queue for execution
     * @param q SYCL queue reference
     */
    void set_queue(sycl::queue& q) {
        queue_ptr = &q;
    }
#else
    // No-op when SYCL not available
    vulkan_parallel_policy() {}
#endif
};

// Global execution policy instance
inline const vulkan_parallel_policy vulkan_par{};

/**
 * @brief Validate functor for GPU execution
 * @tparam Func Functor type
 */
template<typename Func>
constexpr bool is_device_executable() {
    return std::is_trivially_copyable<Func>::value &&
           std::is_trivially_destructible<Func>::value;
}

/**
 * @brief Internal namespace for algorithm implementations
 */
namespace detail {

#ifdef VULKAN_STDPAR_USE_SYCL

/**
 * @brief Execute functor on unified_vector range
 * @tparam T Element type
 * @tparam Func Functor type
 * @param policy Execution policy
 * @param vec Vector to operate on
 * @param start Start index
 * @param count Number of elements
 * @param func Functor to apply
 */
template<typename T, typename Func>
void execute_kernel(const vulkan_parallel_policy& policy,
                   unified_vector<T>& vec,
                   size_t start,
                   size_t count,
                   Func func)
{
    static_assert(is_device_executable<Func>(), 
                  "Functor must be trivially copyable for device execution");
    
    // Get versioning engine
    auto& engine = vec.get_engine();
    
    // Sync to device before execution
    engine.sync_to_device();
    
    // Get SYCL queue
    sycl::queue& q = policy.get_queue();
    
#ifdef VULKAN_STDPAR_ENABLE_PROFILING
    auto start_time = std::chrono::high_resolution_clock::now();
#endif
    
    // Launch kernel
    q.submit([&](sycl::handler& cgh) {
        auto buf = engine.get_device_buffer();
        auto acc = buf.template get_access<sycl::access::mode::read_write>(cgh);
        
        cgh.parallel_for(sycl::range<1>(count), [=](sycl::id<1> idx) {
            size_t i = idx[0] + start;
            func(acc[i]);
        });
    }).wait();
    
#ifdef VULKAN_STDPAR_ENABLE_PROFILING
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    profiling::record_kernel_launch(elapsed.count());
#endif
    
    // Mark device as dirty
    engine.mark_device_dirty();
}

/**
 * @brief Execute transform on unified_vector
 */
template<typename T, typename U, typename Func>
void execute_transform(const vulkan_parallel_policy& policy,
                      unified_vector<T>& input,
                      unified_vector<U>& output,
                      size_t start,
                      size_t count,
                      Func func)
{
    static_assert(is_device_executable<Func>(),
                  "Functor must be trivially copyable for device execution");
    
    auto& input_engine = input.get_engine();
    auto& output_engine = output.get_engine();
    
    // Sync input to device
    input_engine.sync_to_device();
    
    sycl::queue& q = policy.get_queue();
    
#ifdef VULKAN_STDPAR_ENABLE_PROFILING
    auto start_time = std::chrono::high_resolution_clock::now();
#endif
    
    // Launch transform kernel
    q.submit([&](sycl::handler& cgh) {
        auto in_buf = input_engine.get_device_buffer();
        auto out_buf = output_engine.get_device_buffer();
        auto in_acc = in_buf.template get_access<sycl::access::mode::read>(cgh);
        auto out_acc = out_buf.template get_access<sycl::access::mode::write>(cgh);
        
        cgh.parallel_for(sycl::range<1>(count), [=](sycl::id<1> idx) {
            size_t i = idx[0] + start;
            out_acc[i] = func(in_acc[i]);
        });
    }).wait();
    
#ifdef VULKAN_STDPAR_ENABLE_PROFILING
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    profiling::record_kernel_launch(elapsed.count());
#endif
    
    output_engine.mark_device_dirty();
}

/**
 * @brief Execute reduction on unified_vector
 */
template<typename T, typename BinaryOp>
T execute_reduce(const vulkan_parallel_policy& policy,
                unified_vector<T>& vec,
                size_t start,
                size_t count,
                T init,
                BinaryOp op)
{
    static_assert(is_device_executable<BinaryOp>(),
                  "Binary operation must be trivially copyable for device execution");
    
    auto& engine = vec.get_engine();
    engine.sync_to_device();
    
    sycl::queue& q = policy.get_queue();
    
    // Use SYCL reduction
    T* result = sycl::malloc_shared<T>(1, q);
    *result = init;
    
#ifdef VULKAN_STDPAR_ENABLE_PROFILING
    auto start_time = std::chrono::high_resolution_clock::now();
#endif
    
    q.submit([&](sycl::handler& cgh) {
        auto buf = engine.get_device_buffer();
        auto acc = buf.template get_access<sycl::access::mode::read>(cgh);
        auto reduction = sycl::reduction(result, op);
        
        cgh.parallel_for(sycl::range<1>(count), reduction,
                        [=](sycl::id<1> idx, auto& sum) {
            size_t i = idx[0] + start;
            sum.combine(acc[i]);
        });
    }).wait();
    
#ifdef VULKAN_STDPAR_ENABLE_PROFILING
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    profiling::record_kernel_launch(elapsed.count());
#endif
    
    T final_result = *result;
    sycl::free(result, q);
    
    return final_result;
}

#endif // VULKAN_STDPAR_USE_SYCL

} // namespace detail

// ==================== Algorithm Overloads ====================

/**
 * @brief Parallel for_each implementation
 * @tparam T Element type
 * @tparam Func Functor type
 * @param policy Execution policy
 * @param first Beginning of range
 * @param last End of range
 * @param func Unary function to apply
 */
template<typename T, typename Func>
void for_each(const vulkan_parallel_policy& policy,
              typename unified_vector<T>::iterator first,
              typename unified_vector<T>::iterator last,
              Func func)
{
#ifdef VULKAN_STDPAR_USE_SYCL
    auto* container = first.get_container();
    size_t start = first.get_index();
    size_t count = last.get_index() - start;
    
    if (count == 0) return;
    
    detail::execute_kernel(policy, *container, start, count, func);
#else
    // Fallback to CPU execution
    std::for_each(first, last, func);
#endif
}

/**
 * @brief Parallel transform implementation
 * @tparam T Input element type
 * @tparam U Output element type
 * @tparam Func Functor type
 * @param policy Execution policy
 * @param first Beginning of input range
 * @param last End of input range
 * @param d_first Beginning of output range
 * @param func Unary transformation function
 * @return Iterator to end of output range
 */
template<typename T, typename U, typename Func>
typename unified_vector<U>::iterator transform(
    const vulkan_parallel_policy& policy,
    typename unified_vector<T>::const_iterator first,
    typename unified_vector<T>::const_iterator last,
    typename unified_vector<U>::iterator d_first,
    Func func)
{
#ifdef VULKAN_STDPAR_USE_SYCL
    auto* input_container = first.get_container();
    auto* output_container = d_first.get_container();
    size_t start = first.get_index();
    size_t count = last.get_index() - start;
    size_t out_start = d_first.get_index();
    
    if (count == 0) return d_first;
    
    // Ensure output has enough space
    if (output_container->size() < out_start + count) {
        output_container->resize(out_start + count);
    }
    
    detail::execute_transform(policy, 
                             const_cast<unified_vector<T>&>(*input_container),
                             *output_container, start, count, func);
    
    return typename unified_vector<U>::iterator(output_container, out_start + count);
#else
    // Fallback to CPU execution
    return std::transform(first, last, d_first, func);
#endif
}

/**
 * @brief Parallel reduce implementation
 * @tparam T Element type
 * @tparam BinaryOp Binary operation type
 * @param policy Execution policy
 * @param first Beginning of range
 * @param last End of range
 * @param init Initial value
 * @param op Binary operation
 * @return Reduction result
 */
template<typename T, typename BinaryOp = std::plus<T>>
T reduce(const vulkan_parallel_policy& policy,
        typename unified_vector<T>::const_iterator first,
        typename unified_vector<T>::const_iterator last,
        T init = T(),
        BinaryOp op = BinaryOp())
{
#ifdef VULKAN_STDPAR_USE_SYCL
    auto* container = first.get_container();
    size_t start = first.get_index();
    size_t count = last.get_index() - start;
    
    if (count == 0) return init;
    
    return detail::execute_reduce(policy,
                                  const_cast<unified_vector<T>&>(*container),
                                  start, count, init, op);
#else
    // Fallback to CPU execution
    return std::accumulate(first, last, init, op);
#endif
}

/**
 * @brief Parallel sort implementation (uses GPU-optimized sorting)
 * @tparam T Element type
 * @tparam Compare Comparison function type
 * @param policy Execution policy
 * @param first Beginning of range
 * @param last End of range
 * @param comp Comparison function
 */
template<typename T, typename Compare = std::less<T>>
void sort(const vulkan_parallel_policy& policy,
         typename unified_vector<T>::iterator first,
         typename unified_vector<T>::iterator last,
         Compare comp = Compare())
{
#ifdef VULKAN_STDPAR_USE_SYCL
    auto* container = first.get_container();
    size_t start = first.get_index();
    size_t count = last.get_index() - start;
    
    if (count <= 1) return;
    
    // For now, sync to host and use CPU sort
    // TODO: Implement GPU radix/merge sort
    container->get_engine().sync_to_host();
    auto* data = container->get_engine().host_data();
    std::sort(data + start, data + start + count, comp);
    container->get_engine().mark_host_dirty(start, start + count);
#else
    // Fallback to CPU execution
    std::sort(first, last, comp);
#endif
}

} // namespace vulkan_stdpar

// Inject into std namespace for ADL
namespace std {

/**
 * @brief std::for_each overload for vulkan_parallel_policy
 */
template<typename T, typename Func>
void for_each(const vulkan_stdpar::vulkan_parallel_policy& policy,
              typename vulkan_stdpar::unified_vector<T>::iterator first,
              typename vulkan_stdpar::unified_vector<T>::iterator last,
              Func func)
{
    vulkan_stdpar::for_each(policy, first, last, func);
}

/**
 * @brief std::transform overload for vulkan_parallel_policy
 */
template<typename T, typename U, typename Func>
typename vulkan_stdpar::unified_vector<U>::iterator transform(
    const vulkan_stdpar::vulkan_parallel_policy& policy,
    typename vulkan_stdpar::unified_vector<T>::const_iterator first,
    typename vulkan_stdpar::unified_vector<T>::const_iterator last,
    typename vulkan_stdpar::unified_vector<U>::iterator d_first,
    Func func)
{
    return vulkan_stdpar::transform(policy, first, last, d_first, func);
}

/**
 * @brief std::reduce overload for vulkan_parallel_policy
 */
template<typename T, typename BinaryOp = std::plus<T>>
T reduce(const vulkan_stdpar::vulkan_parallel_policy& policy,
        typename vulkan_stdpar::unified_vector<T>::const_iterator first,
        typename vulkan_stdpar::unified_vector<T>::const_iterator last,
        T init = T(),
        BinaryOp op = BinaryOp())
{
    return vulkan_stdpar::reduce(policy, first, last, init, op);
}

/**
 * @brief std::sort overload for vulkan_parallel_policy
 */
template<typename T, typename Compare = std::less<T>>
void sort(const vulkan_stdpar::vulkan_parallel_policy& policy,
         typename vulkan_stdpar::unified_vector<T>::iterator first,
         typename vulkan_stdpar::unified_vector<T>::iterator last,
         Compare comp = Compare())
{
    vulkan_stdpar::sort(policy, first, last, comp);
}

} // namespace std

#endif // VULKAN_STDPAR_ALGORITHMS_PARALLEL_INVOKER_HPP

#endif // VULKAN_STDPAR_VULKAN_STDPAR_HPP
