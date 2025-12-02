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

#ifndef VULKAN_STDPAR_CONTAINERS_UNIFIED_VECTOR_HPP
#define VULKAN_STDPAR_CONTAINERS_UNIFIED_VECTOR_HPP

#include "../core/versioning_engine.hpp"
#include "../core/exceptions.hpp"
#include "unified_reference.hpp"
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
#include "../iterators/unified_iterator.hpp"

#endif // VULKAN_STDPAR_CONTAINERS_UNIFIED_VECTOR_HPP
