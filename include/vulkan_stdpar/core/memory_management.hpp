/**
 * @file memory_management.hpp
 * @brief Memory allocation strategies and optimization utilities
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file contains memory management utilities including pinned memory allocation,
 * lazy buffer creation, and memory optimization strategies.
 */

#ifndef VULKAN_STDPAR_CORE_MEMORY_MANAGEMENT_HPP
#define VULKAN_STDPAR_CORE_MEMORY_MANAGEMENT_HPP

#include <memory>
#include <cstddef>
#include <type_traits>
#include <atomic>
#include <mutex>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>
#endif

namespace vulkan_stdpar {

// Forward declarations
struct device_info;
enum class access_pattern;
enum class allocation_strategy;
enum class memory_advice;

/**
 * @brief Access pattern enumeration for memory optimization
 */
enum class access_pattern {
    sequential,    ///< Sequential access pattern
    random,        ///< Random access pattern
    strided,       ///< Strided access pattern
    unknown        ///< Unknown access pattern
};

/**
 * @brief Memory allocation strategy enumeration
 */
enum class allocation_strategy {
    host_pinned,   ///< Host-pinned memory
    device_local,   ///< Device-local memory
    unified,        ///< Unified memory (if available)
    automatic       ///< Automatic selection based on heuristics
};

/**
 * @brief Memory advice enumeration for optimization hints
 */
enum class memory_advice {
    will_need,           ///< Memory will be needed soon
    will_not_need,        ///< Memory will not be needed
    prefer_location,      ///< Prefer specific location
    preferred_location    ///< Preferred location hint
};

/**
 * @brief Memory properties structure
 */
struct memory_properties {
    bool host_visible;                          ///< Host can access memory
    bool host_coherent;                         ///< Automatic host/device coherence
    bool host_cached;                            ///< Host caching available
    bool device_local;                          ///< Device-local memory
    bool lazily_allocated;                      ///< Lazy allocation support
    size_t alignment;                           ///< Required memory alignment
    size_t max_allocation_size;                 ///< Maximum single allocation
    
    memory_properties()
        : host_visible(false)
        , host_coherent(false)
        , host_cached(false)
        , device_local(false)
        , lazily_allocated(false)
        , alignment(0)
        , max_allocation_size(0)
    {}
};

/**
 * @brief Memory management namespace
 */
namespace memory {

/**
 * @brief Get memory properties for a device
 * @param device Device information
 * @return Memory properties
 */
memory_properties get_memory_properties(const device_info& device);

/**
 * @brief Check if device supports pinned memory
 * @param device Device information
 * @return True if pinned memory is supported
 */
bool supports_pinned_memory(const device_info& device);

/**
 * @brief Check if device supports unified memory
 * @param device Device information
 * @return True if unified memory is supported
 */
bool supports_unified_memory(const device_info& device);

/**
 * @brief Check if device supports device-local memory
 * @param device Device information
 * @return True if device-local memory is supported
 */
bool supports_device_local_memory(const device_info& device);

/**
 * @brief Select optimal allocation strategy
 * @param size Allocation size in bytes
 * @param pattern Expected access pattern
 * @return Optimal allocation strategy
 */
allocation_strategy select_optimal_strategy(size_t size, access_pattern pattern);

/**
 * @brief Select pinned memory allocation strategy
 * @param size Allocation size in bytes
 * @return Pinned memory allocation strategy
 */
allocation_strategy select_pinned_strategy(size_t size);

/**
 * @brief Select device-local memory allocation strategy
 * @param size Allocation size in bytes
 * @return Device-local memory allocation strategy
 */
allocation_strategy select_device_local_strategy(size_t size);

/**
 * @brief Allocate pinned memory
 * @param size Size to allocate in bytes
 * @param device Device information
 * @return Aligned pointer to allocated memory
 * @throws std::bad_alloc if allocation fails
 */
void* allocate_pinned_memory(size_t size, const device_info& device);

/**
 * @brief Allocate device memory
 * @param size Size to allocate in bytes
 * @param device Device information
 * @return Aligned pointer to allocated memory
 * @throws std::bad_alloc if allocation fails
 */
void* allocate_device_memory(size_t size, const device_info& device);

/**
 * @brief Free pinned memory
 * @param ptr Pointer to free
 * @param size Size of allocation
 */
void free_pinned_memory(void* ptr, size_t size);

/**
 * @brief Free device memory
 * @param ptr Pointer to free
 * @param size Size of allocation
 */
void free_device_memory(void* ptr, size_t size);

#ifdef VULKAN_STDPAR_USE_SYCL

/**
 * @brief Lazy SYCL buffer allocator with host-pointer reuse
 * @tparam T Element type
 */
template<typename T>
class lazy_buffer_allocator {
public:
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    
private:
    struct buffer_state {
        sycl::buffer<T> buffer;              ///< SYCL buffer
        T* host_ptr;                        ///< Host pointer
        size_t size;                         ///< Buffer size
        bool device_allocated;                 ///< Device allocation flag
        bool host_ptr_valid;                 ///< Host pointer validity flag
        
        buffer_state() : host_ptr(nullptr), size(0), device_allocated(false), host_ptr_valid(false) {}
    };
    
    mutable std::mutex mutex_;               ///< Thread safety
    mutable std::unique_ptr<buffer_state> state_;  ///< Buffer state
    
public:
    /**
     * @brief Constructor
     */
    lazy_buffer_allocator() : state_(std::make_unique<buffer_state>()) {}
    
    /**
     * @brief Destructor
     */
    ~lazy_buffer_allocator() = default;
    
    // Non-copyable but movable
    lazy_buffer_allocator(const lazy_buffer_allocator&) = delete;
    lazy_buffer_allocator& operator=(const lazy_buffer_allocator&) = delete;
    
    lazy_buffer_allocator(lazy_buffer_allocator&&) = default;
    lazy_buffer_allocator& operator=(lazy_buffer_allocator&&) = default;
    
    /**
     * @brief Get or create SYCL buffer
     * @param host_ptr Host pointer to reuse (optional)
     * @param size Buffer size in elements
     * @return Reference to SYCL buffer
     */
    sycl::buffer<T>& get_or_create_buffer(T* host_ptr = nullptr, size_type size = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!state_->device_allocated || 
            (size > 0 && size != state_->size) ||
            (host_ptr && host_ptr != state_->host_ptr)) {
            
            // Create new buffer with host pointer reuse if possible
            if (host_ptr && size > 0) {
                state_->buffer = sycl::buffer<T>(host_ptr, sycl::range<1>(size));
                state_->host_ptr = host_ptr;
                state_->host_ptr_valid = true;
            } else {
                state_->buffer = sycl::buffer<T>(size);
                state_->host_ptr = nullptr;
                state_->host_ptr_valid = false;
            }
            
            state_->size = size;
            state_->device_allocated = true;
        }
        
        return state_->buffer;
    }
    
    /**
     * @brief Get current buffer size
     * @return Buffer size in elements
     */
    size_type size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_->size;
    }
    
    /**
     * @brief Check if buffer is allocated
     * @return True if buffer is allocated
     */
    bool is_allocated() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_->device_allocated;
    }
    
    /**
     * @brief Get host pointer
     * @return Host pointer (may be nullptr)
     */
    T* get_host_pointer() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_->host_ptr_valid ? state_->host_ptr : nullptr;
    }
    
    /**
     * @brief Reset buffer state
     */
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        state_->buffer = sycl::buffer<T>();
        state_->host_ptr = nullptr;
        state_->size = 0;
        state_->device_allocated = false;
        state_->host_ptr_valid = false;
    }
    
    /**
     * @brief Force buffer allocation
     * @param size Buffer size in elements
     * @return Reference to allocated buffer
     */
    sycl::buffer<T>& allocate(size_type size) {
        return get_or_create_buffer(nullptr, size);
    }
    
    /**
     * @brief Create buffer from existing host data
     * @param host_ptr Host pointer to data
     * @param size Buffer size in elements
     * @return Reference to created buffer
     */
    sycl::buffer<T>& create_from_host(T* host_ptr, size_type size) {
        return get_or_create_buffer(host_ptr, size);
    }
};

/**
 * @brief Optimize memory layout for specific access pattern
 * @tparam T Element type
 * @param buffer SYCL buffer to optimize
 * @param pattern Access pattern
 */
template<typename T>
void optimize_memory_layout(sycl::buffer<T>& buffer, access_pattern pattern);

/**
 * @brief Prefetch memory to specific device
 * @tparam T Element type
 * @param buffer SYCL buffer to prefetch
 * @param device Target device
 */
template<typename T>
void prefetch_memory(sycl::buffer<T>& buffer, const device_info& device);

/**
 * @brief Advise memory usage pattern to runtime
 * @tparam T Element type
 * @param buffer SYCL buffer to advise
 * @param advice Memory advice
 */
template<typename T>
void advise_memory_usage(sycl::buffer<T>& buffer, memory_advice advice);

/**
 * @brief Create sub-buffer for range operations
 * @tparam T Element type
 * @param parent Parent buffer
 * @param offset Offset in elements
 * @param size Size in elements
 * @return Sub-buffer for range
 */
template<typename T>
sycl::buffer<T> create_sub_buffer(sycl::buffer<T>& parent, size_t offset, size_t size) {
    return parent.template reinterpret<T>(sycl::range<1>(offset + size), sycl::id<1>(offset));
}

#endif // VULKAN_STDPAR_USE_SYCL

/**
 * @brief Memory pool for efficient allocation
 * @tparam T Element type
 */
template<typename T>
class memory_pool {
public:
    using value_type = T;
    using size_type = size_t;
    using pointer = T*;
    
private:
    struct block {
        pointer ptr;
        size_type size;
        bool in_use;
        
        block(pointer p = nullptr, size_type s = 0) 
            : ptr(p), size(s), in_use(false) {}
    };
    
    std::vector<block> blocks_;
    std::mutex mutex_;
    size_type total_allocated_;
    size_type peak_usage_;
    
public:
    /**
     * @brief Constructor
     * @param initial_capacity Initial pool capacity
     */
    explicit memory_pool(size_type initial_capacity = 0) 
        : total_allocated_(0), peak_usage_(0) {
        if (initial_capacity > 0) {
            allocate_block(initial_capacity);
        }
    }
    
    /**
     * @brief Destructor
     */
    ~memory_pool() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& block : blocks_) {
            if (block.ptr) {
                operator delete(block.ptr);
            }
        }
    }
    
    /**
     * @brief Allocate memory from pool
     * @param size Size to allocate in elements
     * @return Pointer to allocated memory
     */
    pointer allocate(size_type size) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Try to find a suitable free block
        for (auto& block : blocks_) {
            if (!block.in_use && block.size >= size) {
                block.in_use = true;
                total_allocated_ += size;
                peak_usage_ = std::max(peak_usage_, total_allocated_);
                return block.ptr;
            }
        }
        
        // No suitable block found, allocate new one
        allocate_block(size);
        blocks_.back().in_use = true;
        total_allocated_ += size;
        peak_usage_ = std::max(peak_usage_, total_allocated_);
        return blocks_.back().ptr;
    }
    
    /**
     * @brief Deallocate memory back to pool
     * @param ptr Pointer to deallocate
     * @param size Size of allocation
     */
    void deallocate(pointer ptr, size_type size) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& block : blocks_) {
            if (block.ptr == ptr) {
                block.in_use = false;
                total_allocated_ -= size;
                return;
            }
        }
        
        // Block not found, deallocate directly
        operator delete(ptr);
    }
    
    /**
     * @brief Get current usage statistics
     * @return Total allocated memory
     */
    size_type get_allocated() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return total_allocated_;
    }
    
    /**
     * @brief Get peak usage
     * @return Peak memory usage
     */
    size_type get_peak_usage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return peak_usage_;
    }
    
private:
    void allocate_block(size_type size) {
        pointer ptr = static_cast<pointer>(operator new(size * sizeof(T)));
        blocks_.emplace_back(ptr, size);
    }
};

} // namespace memory

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_MEMORY_MANAGEMENT_HPP