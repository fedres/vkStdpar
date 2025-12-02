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

#ifndef VULKAN_STDPAR_ALGORITHMS_PARALLEL_INVOKER_HPP
#define VULKAN_STDPAR_ALGORITHMS_PARALLEL_INVOKER_HPP

#include "../core/versioning_engine.hpp"
#include "../core/device_selection.hpp"
#include "../core/profiling.hpp"
#include "../core/exceptions.hpp"
#include "../containers/unified_vector.hpp"
#include <algorithm>
#include <numeric>
#include <functional>
#include <type_traits>

#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>
#endif

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
