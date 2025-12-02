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

#ifndef VULKAN_STDPAR_CORE_VERSIONING_ENGINE_HPP
#define VULKAN_STDPAR_CORE_VERSIONING_ENGINE_HPP

#include <atomic>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <algorithm>
#include <cassert>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>
#endif

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