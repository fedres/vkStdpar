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

#ifndef VULKAN_STDPAR_CONTAINERS_UNIFIED_REFERENCE_HPP
#define VULKAN_STDPAR_CONTAINERS_UNIFIED_REFERENCE_HPP

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

#endif // VULKAN_STDPAR_CONTAINERS_UNIFIED_REFERENCE_HPP
