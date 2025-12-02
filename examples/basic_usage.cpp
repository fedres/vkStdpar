/**
 * @file basic_example.cpp
 * @brief Simple example demonstrating unified_vector usage
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 */

#include <vulkan_stdpar/vulkan_stdpar.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>

int main() {
    std::cout << "Vulkan STD-Parallel Basic Example\n";
    std::cout << "==================================\n\n";
    
    // Create a unified_vector (drop-in replacement for std::vector)
    vulkan_stdpar::unified_vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    
    std::cout << "Original data: ";
    for (int v : data) {
        std::cout << v << " ";
    }
    std::cout << "\n\n";
    
    // Sort (CPU fallback when SYCL not available)
    std::cout << "Sorting data...\\n";
    std::sort(data.begin(), data.end());
    
    std::cout << "Sorted data: ";
    for (int v : data) {
        std::cout << v << " ";
    }
    std::cout << "\n\n";
    
    // Transform example
    vulkan_stdpar::unified_vector<int> transformed(data.size());
    std::cout << "Transforming (multiply by 2)...\n";
    
#ifdef VULKAN_STDPAR_USE_SYCL
    vulkan_stdpar::transform(vulkan_stdpar::vulkan_par,
                              data.begin(), data.end(),
                              transformed.begin(),
                              [](int x) { return x * 2; });
#else
    std::transform(data.begin(), data.end(), transformed.begin(),
                  [](int x) { return x * 2; });
#endif
    
    std::cout << "Transformed data: ";
    for (int v : transformed) {
        std::cout << v << " ";
    }
    std::cout << "\n\n";
    
    // Reduce example
    std::cout << "Computing sum with std::reduce...\n";
#ifdef VULKAN_STDPAR_USE_SYCL
    int sum = vulkan_stdpar::reduce(vulkan_stdpar::vulkan_par,
                                     transformed.begin(),
                                     transformed.end(),
                                     0,
                                     std::plus<int>());
#else
    int sum = std::accumulate(transformed.begin(), transformed.end(), 0);
#endif
    
    std::cout << "Sum: " << sum << "\n\n";
    
    // Element modification example
    std::cout << "Modifying first element...\n";
    data[0] = 100;
    std::cout << "First element now: " << data[0] << "\n\n";
    
    // Capacity operations
    std::cout << "Vector operations:\n";
    std::cout << "  Size: " << data.size() << "\n";
    std::cout << "  Capacity: " << data.capacity() << "\n";
    std::cout << "  Empty: " << (data.empty() ? "yes" : "no") << "\n\n";
    
    data.push_back(99);
    std::cout << "After push_back(99), size: " << data.size() << "\n";
    std::cout << "Last element: " << data.back() << "\n\n";
    
    std::cout << "Example completed successfully!\n";
    
    return 0;
}
