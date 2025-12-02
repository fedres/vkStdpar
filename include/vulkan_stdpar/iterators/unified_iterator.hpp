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

#ifndef VULKAN_STDPAR_ITERATORS_UNIFIED_ITERATOR_HPP
#define VULKAN_STDPAR_ITERATORS_UNIFIED_ITERATOR_HPP

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

#endif // VULKAN_STDPAR_ITERATORS_UNIFIED_ITERATOR_HPP
