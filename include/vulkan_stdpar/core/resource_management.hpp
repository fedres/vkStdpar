/**
 * @file resource_management.hpp
 * @brief RAII resource management for GPU buffers and kernels
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file contains RAII wrappers for GPU resources to ensure
 * proper cleanup and exception safety.
 */

#ifndef VULKAN_STDPAR_CORE_RESOURCE_MANAGEMENT_HPP
#define VULKAN_STDPAR_CORE_RESOURCE_MANAGEMENT_HPP

#include <memory>
#include <utility>
#include <type_traits>
#include <functional>
#include <mutex>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>
#endif

namespace vulkan_stdpar {

#ifdef VULKAN_STDPAR_USE_SYCL

/**
 * @brief RAII wrapper for SYCL buffer
 * @tparam T Element type
 */
template<typename T>
class buffer_guard {
public:
    using value_type = T;
    using buffer_type = sycl::buffer<T>;
    using pointer = T*;
    using size_type = size_t;
    
private:
    std::unique_ptr<buffer_type> buffer_;
    bool owns_buffer_;
    
public:
    /**
     * @brief Default constructor
     */
    buffer_guard() : buffer_(nullptr), owns_buffer_(false) {}
    
    /**
     * @brief Construct with existing buffer (non-owning)
     * @param buffer Existing buffer
     */
    explicit buffer_guard(buffer_type& buffer) 
        : buffer_(&buffer), owns_buffer_(false) {}
    
    /**
     * @brief Construct with new buffer (owning)
     * @param size Buffer size in elements
     */
    explicit buffer_guard(size_type size) 
        : buffer_(std::make_unique<buffer_type>(size)), owns_buffer_(true) {}
    
    /**
     * @brief Construct with host pointer (owning)
     * @param host_ptr Host pointer
     * @param size Buffer size in elements
     */
    buffer_guard(pointer host_ptr, size_type size) 
        : buffer_(std::make_unique<buffer_type>(host_ptr, sycl::range<1>(size))), owns_buffer_(true) {}
    
    /**
     * @brief Move constructor
     */
    buffer_guard(buffer_guard&& other) noexcept
        : buffer_(std::move(other.buffer_)), owns_buffer_(other.owns_buffer_) {
        other.owns_buffer_ = false;
    }
    
    /**
     * @brief Move assignment
     */
    buffer_guard& operator=(buffer_guard&& other) noexcept {
        if (this != &other) {
            reset();
            buffer_ = std::move(other.buffer_);
            owns_buffer_ = other.owns_buffer_;
            other.owns_buffer_ = false;
        }
        return *this;
    }
    
    /**
     * @brief Destructor
     */
    ~buffer_guard() {
        reset();
    }
    
    // Non-copyable
    buffer_guard(const buffer_guard&) = delete;
    buffer_guard& operator=(const buffer_guard&) = delete;
    
    /**
     * @brief Get underlying buffer
     * @return Pointer to buffer (may be nullptr)
     */
    buffer_type* get() const noexcept {
        return buffer_.get();
    }
    
    /**
     * @brief Get underlying buffer
     * @return Reference to buffer
     */
    buffer_type& operator*() const {
        return *buffer_;
    }
    
    /**
     * @brief Get underlying buffer
     * @return Pointer to buffer
     */
    buffer_type* operator->() const noexcept {
        return buffer_.get();
    }
    
    /**
     * @brief Check if buffer is valid
     * @return True if buffer is valid
     */
    explicit operator bool() const noexcept {
        return buffer_ != nullptr;
    }
    
    /**
     * @brief Release ownership of buffer
     * @return Raw pointer to buffer
     */
    buffer_type* release() noexcept {
        owns_buffer_ = false;
        return buffer_.release();
    }
    
    /**
     * @brief Reset to empty state
     */
    void reset() {
        if (owns_buffer_ && buffer_) {
            buffer_.reset();
        }
        owns_buffer_ = false;
    }
    
    /**
     * @brief Reset with new buffer
     * @param size New buffer size in elements
     */
    void reset(size_type size) {
        reset();
        buffer_ = std::make_unique<buffer_type>(size);
        owns_buffer_ = true;
    }
    
    /**
     * @brief Reset with host pointer
     * @param host_ptr Host pointer
     * @param size Buffer size in elements
     */
    void reset(pointer host_ptr, size_type size) {
        reset();
        buffer_ = std::make_unique<buffer_type>(host_ptr, sycl::range<1>(size));
        owns_buffer_ = true;
    }
};

/**
 * @brief RAII wrapper for SYCL queue
 */
class queue_guard {
public:
    using queue_type = sycl::queue;
    
private:
    std::unique_ptr<queue_type> queue_;
    bool owns_queue_;
    
public:
    /**
     * @brief Default constructor
     */
    queue_guard() : queue_(nullptr), owns_queue_(false) {}
    
    /**
     * @brief Construct with existing queue (non-owning)
     * @param queue Existing queue
     */
    explicit queue_guard(queue_type& queue) 
        : queue_(&queue), owns_queue_(false) {}
    
    /**
     * @brief Construct with device selector (owning)
     * @param selector Device selector
     */
    explicit queue_guard(const sycl::device_selector& selector) 
        : queue_(std::make_unique<queue_type>(selector)), owns_queue_(true) {}
    
    /**
     * @brief Move constructor
     */
    queue_guard(queue_guard&& other) noexcept
        : queue_(std::move(other.queue_)), owns_queue_(other.owns_queue_) {
        other.owns_queue_ = false;
    }
    
    /**
     * @brief Move assignment
     */
    queue_guard& operator=(queue_guard&& other) noexcept {
        if (this != &other) {
            reset();
            queue_ = std::move(other.queue_);
            owns_queue_ = other.owns_queue_;
            other.owns_queue_ = false;
        }
        return *this;
    }
    
    /**
     * @brief Destructor
     */
    ~queue_guard() {
        reset();
    }
    
    // Non-copyable
    queue_guard(const queue_guard&) = delete;
    queue_guard& operator=(const queue_guard&) = delete;
    
    /**
     * @brief Get underlying queue
     * @return Pointer to queue (may be nullptr)
     */
    queue_type* get() const noexcept {
        return queue_.get();
    }
    
    /**
     * @brief Get underlying queue
     * @return Reference to queue
     */
    queue_type& operator*() const {
        return *queue_;
    }
    
    /**
     * @brief Get underlying queue
     * @return Pointer to queue
     */
    queue_type* operator->() const noexcept {
        return queue_.get();
    }
    
    /**
     * @brief Check if queue is valid
     * @return True if queue is valid
     */
    explicit operator bool() const noexcept {
        return queue_ != nullptr;
    }
    
    /**
     * @brief Release ownership of queue
     * @return Raw pointer to queue
     */
    queue_type* release() noexcept {
        owns_queue_ = false;
        return queue_.release();
    }
    
    /**
     * @brief Reset to empty state
     */
    void reset() {
        if (owns_queue_ && queue_) {
            queue_->wait_and_throw();
            queue_.reset();
        }
        owns_queue_ = false;
    }
    
    /**
     * @brief Reset with new queue
     * @param selector Device selector
     */
    void reset(const sycl::device_selector& selector) {
        reset();
        queue_ = std::make_unique<queue_type>(selector);
        owns_queue_ = true;
    }
};

/**
 * @brief RAII wrapper for SYCL event
 */
class event_guard {
public:
    using event_type = sycl::event;
    
private:
    std::unique_ptr<event_type> event_;
    bool owns_event_;
    
public:
    /**
     * @brief Default constructor
     */
    event_guard() : event_(nullptr), owns_event_(false) {}
    
    /**
     * @brief Construct with existing event (non-owning)
     * @param event Existing event
     */
    explicit event_guard(event_type& event) 
        : event_(&event), owns_event_(false) {}
    
    /**
     * @brief Move constructor
     */
    event_guard(event_guard&& other) noexcept
        : event_(std::move(other.event_)), owns_event_(other.owns_event_) {
        other.owns_event_ = false;
    }
    
    /**
     * @brief Move assignment
     */
    event_guard& operator=(event_guard&& other) noexcept {
        if (this != &other) {
            reset();
            event_ = std::move(other.event_);
            owns_event_ = other.owns_event_;
            other.owns_event_ = false;
        }
        return *this;
    }
    
    /**
     * @brief Destructor
     */
    ~event_guard() {
        reset();
    }
    
    // Non-copyable
    event_guard(const event_guard&) = delete;
    event_guard& operator=(const event_guard&) = delete;
    
    /**
     * @brief Get underlying event
     * @return Pointer to event (may be nullptr)
     */
    event_type* get() const noexcept {
        return event_.get();
    }
    
    /**
     * @brief Get underlying event
     * @return Reference to event
     */
    event_type& operator*() const {
        return *event_;
    }
    
    /**
     * @brief Get underlying event
     * @return Pointer to event
     */
    event_type* operator->() const noexcept {
        return event_.get();
    }
    
    /**
     * @brief Check if event is valid
     * @return True if event is valid
     */
    explicit operator bool() const noexcept {
        return event_ != nullptr;
    }
    
    /**
     * @brief Wait for event completion
     */
    void wait() const {
        if (event_) {
            event_->wait();
        }
    }
    
    /**
     * @brief Release ownership of event
     * @return Raw pointer to event
     */
    event_type* release() noexcept {
        owns_event_ = false;
        return event_.release();
    }
    
    /**
     * @brief Reset to empty state
     */
    void reset() {
        if (owns_event_ && event_) {
            event_->wait();
            event_.reset();
        }
        owns_event_ = false;
    }
};

/**
 * @brief RAII wrapper for kernel execution context
 */
class kernel_guard {
public:
    using queue_type = sycl::queue;
    using event_type = sycl::event;
    
private:
    queue_guard queue_;
    std::vector<event_guard> events_;
    std::mutex mutex_;
    
public:
    /**
     * @brief Constructor
     * @param queue SYCL queue for kernel execution
     */
    explicit kernel_guard(queue_type& queue) : queue_(queue) {}
    
    /**
     * @brief Constructor with device selector
     * @param selector Device selector
     */
    explicit kernel_guard(const sycl::device_selector& selector) : queue_(selector) {}
    
    /**
     * @brief Destructor - waits for all events
     */
    ~kernel_guard() {
        wait_all();
    }
    
    // Non-copyable but movable
    kernel_guard(const kernel_guard&) = delete;
    kernel_guard& operator=(const kernel_guard&) = delete;
    
    kernel_guard(kernel_guard&&) = default;
    kernel_guard& operator=(kernel_guard&&) = default;
    
    /**
     * @brief Get queue
     * @return Reference to queue
     */
    queue_type& get_queue() const {
        return *queue_;
    }
    
    /**
     * @brief Submit kernel and track event
     * @param command_group Command group function
     * @return Event for submitted kernel
     */
    template<typename CommandGroup>
    event_type submit(CommandGroup&& command_group) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        event_type event = queue_->submit(std::forward<CommandGroup>(command_group));
        events_.emplace_back(event);
        
        return event;
    }
    
    /**
     * @brief Wait for all submitted kernels
     */
    void wait_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& event : events_) {
            event.wait();
        }
        events_.clear();
    }
    
    /**
     * @brief Get number of pending events
     * @return Number of pending events
     */
    size_t pending_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_.size();
    }
    
    /**
     * @brief Check if any kernels are pending
     * @return True if kernels are pending
     */
    bool has_pending() const {
        return pending_count() > 0;
    }
};

/**
 * @brief RAII wrapper for memory access scope
 * @tparam BufferType Buffer type
 * @tparam AccessMode Access mode
 * @tparam AccessTarget Access target
 */
template<typename BufferType, sycl::access::mode AccessMode, sycl::access::target AccessTarget>
class access_guard {
public:
    using accessor_type = sycl::accessor<typename BufferType::value_type, 1, AccessMode, AccessTarget>;
    
private:
    std::unique_ptr<accessor_type> accessor_;
    BufferType* buffer_;
    bool has_access_;
    
public:
    /**
     * @brief Constructor
     * @param buffer Buffer to access
     * @param command_group_handler Command group handler
     */
    access_guard(BufferType& buffer, sycl::handler& command_group_handler)
        : buffer_(&buffer), has_access_(false) {
        accessor_ = std::make_unique<accessor_type>(buffer, command_group_handler);
        has_access_ = true;
    }
    
    /**
     * @brief Move constructor
     */
    access_guard(access_guard&& other) noexcept
        : accessor_(std::move(other.accessor_))
        , buffer_(other.buffer_)
        , has_access_(other.has_access_) {
        other.has_access_ = false;
    }
    
    /**
     * @brief Move assignment
     */
    access_guard& operator=(access_guard&& other) noexcept {
        if (this != &other) {
            accessor_ = std::move(other.accessor_);
            buffer_ = other.buffer_;
            has_access_ = other.has_access_;
            other.has_access_ = false;
        }
        return *this;
    }
    
    /**
     * @brief Destructor
     */
    ~access_guard() = default;
    
    // Non-copyable
    access_guard(const access_guard&) = delete;
    access_guard& operator=(const access_guard&) = delete;
    
    /**
     * @brief Get accessor
     * @return Reference to accessor
     */
    accessor_type& get() {
        return *accessor_;
    }
    
    /**
     * @brief Get accessor
     * @return Reference to accessor
     */
    accessor_type& operator*() {
        return *accessor_;
    }
    
    /**
     * @brief Get accessor
     * @return Pointer to accessor
     */
    accessor_type* operator->() {
        return accessor_.get();
    }
    
    /**
     * @brief Check if access is valid
     * @return True if access is valid
     */
    explicit operator bool() const noexcept {
        return has_access_ && accessor_ != nullptr;
    }
};

#endif // VULKAN_STDPAR_USE_SYCL

/**
 * @brief Generic RAII resource guard
 * @tparam Resource Resource type
 * @tparam Deleter Deleter function type
 */
template<typename Resource, typename Deleter>
class resource_guard {
public:
    using resource_type = Resource;
    using deleter_type = Deleter;
    
private:
    Resource resource_;
    Deleter deleter_;
    bool owns_resource_;
    
public:
    /**
     * @brief Constructor
     * @param resource Resource to guard
     * @param deleter Deleter function
     * @param owns Whether this guard owns the resource
     */
    resource_guard(Resource resource, Deleter deleter, bool owns = true)
        : resource_(resource), deleter_(deleter), owns_resource_(owns) {}
    
    /**
     * @brief Move constructor
     */
    resource_guard(resource_guard&& other) noexcept
        : resource_(std::move(other.resource_))
        , deleter_(std::move(other.deleter_))
        , owns_resource_(other.owns_resource_) {
        other.owns_resource_ = false;
    }
    
    /**
     * @brief Move assignment
     */
    resource_guard& operator=(resource_guard&& other) noexcept {
        if (this != &other) {
            cleanup();
            resource_ = std::move(other.resource_);
            deleter_ = std::move(other.deleter_);
            owns_resource_ = other.owns_resource_;
            other.owns_resource_ = false;
        }
        return *this;
    }
    
    /**
     * @brief Destructor
     */
    ~resource_guard() {
        cleanup();
    }
    
    // Non-copyable
    resource_guard(const resource_guard&) = delete;
    resource_guard& operator=(const resource_guard&) = delete;
    
    /**
     * @brief Get resource
     * @return Reference to resource
     */
    Resource& get() noexcept {
        return resource_;
    }
    
    /**
     * @brief Get resource
     * @return Const reference to resource
     */
    const Resource& get() const noexcept {
        return resource_;
    }
    
    /**
     * @brief Release ownership
     * @return The resource
     */
    Resource release() noexcept {
        owns_resource_ = false;
        return std::move(resource_);
    }
    
    /**
     * @brief Reset with new resource
     * @param resource New resource
     */
    void reset(Resource resource) {
        cleanup();
        resource_ = std::move(resource);
        owns_resource_ = true;
    }
    
private:
    void cleanup() {
        if (owns_resource_) {
            deleter_(resource_);
        }
    }
};

/**
 * @brief Factory function for resource guards
 * @tparam Resource Resource type
 * @tparam Deleter Deleter type
 * @param resource Resource to guard
 * @param deleter Deleter function
 * @return Resource guard
 */
template<typename Resource, typename Deleter>
resource_guard<Resource, Deleter> make_resource_guard(Resource resource, Deleter deleter) {
    return resource_guard<Resource, Deleter>(std::move(resource), std::move(deleter));
}

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_RESOURCE_MANAGEMENT_HPP