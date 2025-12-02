# Vulkan STD-Parallel API Documentation

## Table of Contents

1. [Core Components](#core-components)
2. [Containers](#containers)
3. [Algorithms](#algorithms)
4. [Device Management](#device-management)
5. [Performance Profiling](#performance-profiling)

---

## Core Components

### `vulkan_parallel_policy`

Execution policy for GPU-accelerated algorithms.

```cpp
namespace vulkan_stdpar {
    struct vulkan_parallel_policy {
        vulkan_parallel_policy();
        explicit vulkan_parallel_policy(sycl::queue& q);
    };
    
    // Global instance
    inline const vulkan_parallel_policy vulkan_par{};
}
```

**Usage:**
```cpp
std::sort(vulkan_stdpar::vulkan_par, vec.begin(), vec.end());
```

---

## Containers

### `unified_vector<T>`

Drop-in replacement for `std::vector<T>` with automatic GPU memory management.

#### Constructors

```cpp
unified_vector();                                    // Default
unified_vector(size_t count);                        // Size
unified_vector(size_t count, const T& value);        // Fill
unified_vector(InputIt first, InputIt last);         // Range
unified_vector(std::initializer_list<T> init);       // Initializer list
unified_vector(const unified_vector& other);         // Copy
unified_vector(unified_vector&& other) noexcept;     // Move
```

#### Element Access

```cpp
T& operator[](size_t pos);
const T& operator[](size_t pos) const;
T& at(size_t pos);
const T& at(size_t pos) const;
T& front();
T& back();
T* data();
```

#### Capacity

```cpp
bool empty() const noexcept;
size_t size() const noexcept;
size_t capacity() const noexcept;
void reserve(size_t new_cap);
void resize(size_t count);
void resize(size_t count, const T& value);
void shrink_to_fit();
```

#### Modifiers

```cpp
void clear() noexcept;
void push_back(const T& value);
void push_back(T&& value);
void pop_back();
iterator insert(const_iterator pos, const T& value);
iterator erase(const_iterator pos);
void swap(unified_vector& other) noexcept;
```

#### Iterators

```cpp
iterator begin() noexcept;
const_iterator begin() const noexcept;
iterator end() noexcept;
const_iterator end() const noexcept;
```

**Example:**
```cpp
vulkan_stdpar::unified_vector<float> vec = {1.0f, 2.0f, 3.0f};
vec.push_back(4.0f);
vec.resize(10);
float val = vec[0];
```

---

## Algorithms

All algorithms support both `vulkan_parallel_policy` for GPU execution and standard execution.

### `for_each`

Apply function to each element.

```cpp
template<typename T, typename Func>
void for_each(const vulkan_parallel_policy& policy,
              typename unified_vector<T>::iterator first,
              typename unified_vector<T>::iterator last,
              Func func);
```

**Example:**
```cpp
vulkan_stdpar::unified_vector<int> vec = {1, 2, 3, 4, 5};
std::for_each(vulkan_stdpar::vulkan_par, vec.begin(), vec.end(),
              [](auto&& x) { x *= 2; });
```

### `transform`

Transform elements from input to output range.

```cpp
template<typename T, typename U, typename Func>
typename unified_vector<U>::iterator transform(
    const vulkan_parallel_policy& policy,
    typename unified_vector<T>::const_iterator first,
    typename unified_vector<T>::const_iterator last,
    typename unified_vector<U>::iterator d_first,
    Func func);
```

**Example:**
```cpp
vulkan_stdpar::unified_vector<int> input = {1, 2, 3, 4, 5};
vulkan_stdpar::unified_vector<int> output(input.size());
std::transform(vulkan_stdpar::vulkan_par, 
               input.begin(), input.end(), output.begin(),
               [](int x) { return x * x; });
```

### `reduce`

Parallel reduction/accumulation.

```cpp
template<typename T, typename BinaryOp = std::plus<T>>
T reduce(const vulkan_parallel_policy& policy,
         typename unified_vector<T>::const_iterator first,
         typename unified_vector<T>::const_iterator last,
         T init = T(),
         BinaryOp op = BinaryOp());
```

**Example:**
```cpp
vulkan_stdpar::unified_vector<int> vec = {1, 2, 3, 4, 5};
int sum = std::reduce(vulkan_stdpar::vulkan_par, 
                      vec.begin(), vec.end(), 0);
```

### `sort`

GPU-optimized sorting.

```cpp
template<typename T, typename Compare = std::less<T>>
void sort(const vulkan_parallel_policy& policy,
          typename unified_vector<T>::iterator first,
          typename unified_vector<T>::iterator last,
          Compare comp = Compare());
```

**Example:**
```cpp
vulkan_stdpar::unified_vector<int> vec = {5, 2, 8, 1, 9};
std::sort(vulkan_stdpar::vulkan_par, vec.begin(), vec.end());
```

---

## Device Management

### Device Enumeration

```cpp
namespace vulkan_stdpar::device {
    // Get all available devices
    std::vector<device_info> enumerate_devices();
    
    // Get suitable devices for compute
    std::vector<device_info> enumerate_suitable_devices();
    
    // Get default device
    device_info get_default_device();
}
```

### Device Selection

```cpp
namespace vulkan_stdpar::device {
    // Select by name
    device_info select_by_name(const std::string& device_name);
    
    // Select by vendor
    device_info select_by_vendor(const std::string& vendor_name);
    
    // Select by memory requirement
    device_info select_by_memory(size_t minimum_memory);
    
    // Select by performance score
    device_info select_by_performance(double min_score);
}
```

### Device Ranking

```cpp
namespace vulkan_stdpar::device {
    // Rank by performance
    std::vector<device_info> rank_devices_by_performance();
    
    // Rank by memory
    std::vector<device_info> rank_devices_by_memory();
}
```

### `device_info` Structure

```cpp
struct device_info {
    std::string name;
    std::string vendor;
    size_t memory_size;
    size_t max_compute_units;
    size_t max_work_group_size;
    bool supports_fp16;
    bool supports_fp64;
    
    bool is_suitable() const;
    double performance_score() const;
    std::string get_summary() const;
};
```

**Example:**
```cpp
auto devices = vulkan_stdpar::device::enumerate_devices();
for (const auto& dev : devices) {
    std::cout << dev.name << " - " << dev.vendor << std::endl;
    std::cout << "  Memory: " << (dev.memory_size / (1024*1024*1024)) << " GB\n";
    std::cout << "  Score: " << dev.performance_score() << std::endl;
}
```

---

## Performance Profiling

Enable profiling at compile time:

```cpp
#define VULKAN_STDPAR_ENABLE_PROFILING
#include <vulkan_stdpar/vulkan_stdpar.hpp>
```

### Profiling API

```cpp
namespace vulkan_stdpar::profiling {
    // Get performance statistics
    performance_stats get_statistics();
    
    // Reset counters
    void reset_counters();
    
    // Scoped timer
    class scoped_timer {
    public:
        scoped_timer(const std::string& name);
        ~scoped_timer();
    };
}
```

**Example:**
```cpp
#define VULKAN_STDPAR_ENABLE_PROFILING
#include <vulkan_stdpar/vulkan_stdpar.hpp>

{
    vulkan_stdpar::profiling::scoped_timer timer("sort_operation");
    std::sort(vulkan_stdpar::vulkan_par, vec.begin(), vec.end());
}

auto stats = vulkan_stdpar::profiling::get_statistics();
std::cout << "Kernel launches: " << stats.kernel_launches << std::endl;
```

---

## Error Handling

### Exception Hierarchy

```cpp
vulkan_stdpar_exception                    // Base exception
├── synchronization_exception              // Sync errors
├── compilation_exception                  // Kernel compilation errors
├── out_of_memory_exception               // Memory allocation errors
├── device_not_found_exception            // Device selection errors
└── device_lost_exception                 // Device disconnection
```

**Example:**
```cpp
try {
    auto dev = vulkan_stdpar::device::select_by_name("NonExistent");
} catch (const vulkan_stdpar::device_not_found_exception& e) {
    std::cerr << "Device error: " << e.what() << std::endl;
}
```

---

## Best Practices

### 1. Lambda Requirements

Lambdas must be trivially copyable for GPU execution:

```cpp
// ✅ Good - trivially copyable
std::transform(vulkan_stdpar::vulkan_par, vec.begin(), vec.end(), out.begin(),
               [](int x) { return x * 2; });

// ❌ Bad - captures non-trivial state
int multiplier = 2;
std::transform(vulkan_stdpar::vulkan_par, vec.begin(), vec.end(), out.begin(),
               [&multiplier](int x) { return x * multiplier; });  // Won't compile
```

### 2. Memory Management

The library handles synchronization automatically, but you can control it:

```cpp
vulkan_stdpar::unified_vector<int> vec(1000);

// Automatic sync on access
vec[0] = 42;  // Syncs to host if needed

// Manual control (advanced)
vec.get_engine().sync_to_device();  // Force sync to GPU
vec.get_engine().sync_to_host();    // Force sync to CPU
```

### 3. Performance Tips

- Use `reserve()` to pre-allocate memory
- Batch operations to minimize sync overhead
- Prefer `transform` over `for_each` for pure transformations
- Use `reduce` instead of manual accumulation

```cpp
// Good - single operation
vec.reserve(1000000);
std::transform(vulkan_stdpar::vulkan_par, in.begin(), in.end(), out.begin(), func);

// Less efficient - multiple syncs
for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = func(vec[i]);  // Syncs on each access
}
```

---

## Platform Support

| Platform | SYCL Support | Status |
|----------|-------------|--------|
| Linux + Intel GPU | ✅ | Full support |
| Linux + NVIDIA GPU | ✅ | Full support (via CUDA backend) |
| Linux + AMD GPU | ✅ | Full support (via ROCm backend) |
| Windows + Intel GPU | ✅ | Full support |
| macOS (Intel) | ⚠️ | CPU fallback |
| macOS (Apple Silicon) | ⚠️ | CPU fallback |

CPU fallback provides full API compatibility with standard algorithm performance.
