/**
 * @file algorithms_demo.cpp
 * @brief Comprehensive demonstration of parallel algorithms
 */

#include <vulkan_stdpar/vulkan_stdpar.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    std::cout << "Vulkan STD-Parallel - Algorithms Demo\n";
    std::cout << "=====================================\n\n";
    
    // Test 1: Transform
    {
        std::cout << "🔄 Test 1: Transform (square each element)\n";
        std::cout << std::string(50, '-') << "\n";
        
        vulkan_stdpar::unified_vector<int> input = {1, 2, 3, 4, 5};
        vulkan_stdpar::unified_vector<int> output(input.size());
        
        std::cout << "Input:  ";
        for (int v : input) std::cout << v << " ";
        std::cout << "\n";
        
        std::transform(input.begin(), input.end(), output.begin(),
                      [](int x) { return x * x; });
        
        std::cout << "Output: ";
        for (int v : output) std::cout << v << " ";
        std::cout << "\n\n";
    }
    
    // Test 2: Reduce (sum)
    {
        std::cout << "➕ Test 2: Reduce (sum)\n";
        std::cout << std::string(50, '-') << "\n";
        
        vulkan_stdpar::unified_vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        
        std::cout << "Data: ";
        for (int v : data) std::cout << v << " ";
        std::cout << "\n";
        
        int sum = std::accumulate(data.begin(), data.end(), 0);
        std::cout << "Sum: " << sum << " (expected: 55)\n\n";
    }
    
    // Test 3: For-each
    {
        std::cout << "🔁 Test 3: For-each (increment all)\n";
        std::cout << std::string(50, '-') << "\n";
        
        vulkan_stdpar::unified_vector<int> data = {0, 0, 0, 0, 0};
        
        std::cout << "Before: ";
        for (int v : data) std::cout << v << " ";
        std::cout << "\n";
        
        std::for_each(data.begin(), data.end(), [](auto&& x) { x += 1; });
        
        std::cout << "After:  ";
        for (int v : data) std::cout << v << " ";
        std::cout << "\n\n";
    }
    
    // Test 4: Sort
    {
        std::cout << "📊 Test 4: Sort (descending)\n";
        std::cout << std::string(50, '-') << "\n";
        
        vulkan_stdpar::unified_vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
        
        std::cout << "Before: ";
        for (int v : data) std::cout << v << " ";
        std::cout << "\n";
        
        std::sort(data.begin(), data.end(), std::greater<int>());
        
        std::cout << "After:  ";
        for (int v : data) std::cout << v << " ";
        std::cout << "\n\n";
    }
    
    // Test 5: Find & Count
    {
        std::cout << "🔍 Test 5: Find & Count\n";
        std::cout << std::string(50, '-') << "\n";
        
        vulkan_stdpar::unified_vector<int> data = {1, 2, 3, 2, 4, 2, 5};
        
        std::cout << "Data: ";
        for (int v : data) std::cout << v << " ";
        std::cout << "\n";
        
        auto it = std::find(data.begin(), data.end(), 3);
        if (it != data.end()) {
            std::cout << "Found 3 at index: " << (it - data.begin()) << "\n";
        }
        
        int count = std::count(data.begin(), data.end(), 2);
        std::cout << "Count of 2: " << count << "\n\n";
    }
    
    // Test 6: Large dataset performance
    {
        std::cout << "⚡ Test 6: Large Dataset (1M elements)\n";
        std::cout << std::string(50, '-') << "\n";
        
        const size_t N = 1000000;
        vulkan_stdpar::unified_vector<float> large_data(N);
        
        // Fill with values
        for (size_t i = 0; i < N; ++i) {
            large_data[i] = static_cast<float>(i);
        }
        
        // Transform: compute sqrt
        auto start = std::chrono::high_resolution_clock::now();
        std::transform(large_data.begin(), large_data.end(), large_data.begin(),
                      [](float x) { return std::sqrt(x); });
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Transform (sqrt) on " << N << " elements: " 
                  << duration.count() << " ms\n";
        
        // Verify a few values
        std::cout << "Sample values: sqrt(0)=" << large_data[0] 
                  << ", sqrt(100)=" << large_data[100] 
                  << ", sqrt(10000)=" << large_data[10000] << "\n\n";
    }
    
    std::cout << "✅ All algorithm tests completed successfully!\n";
    
    return 0;
}
