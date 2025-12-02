# Vulkan STD-Parallel v1.0

A header-only C++17 library providing GPU-accelerated drop-in replacements for `std::vector` and standard parallel algorithms using SYCL/Vulkan.

## Features

- **Drop-in `std::vector` replacement**: `unified_vector<T>` with 100% API compatibility
- **GPU-accelerated algorithms**: `for_each`, `transform`, `reduce`, `sort` with automatic GPU dispatch
- **Transparent memory management**: Automatic host/device synchronization with dirty tracking
- **Cross-platform**: SYCL backend with CPU fallback when GPU unavailable
- **Header-only**: No compilation required, just include and use
- **Thread-safe**: Built-in synchronization for concurrent access

## Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16+ (for examples)
- Vulkan SDK 1.3+ (optional, for GPU acceleration)
- SYCL 2020 runtime (optional, Sylkan recommended)

### Installation

Header-only library - just copy `include/vulkan_stdpar/` to your project:

```bash
# Copy headers to your include path
cp -r include/vulkan_stdpar /usr/local/include/

# Or use CMake
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
sudo make install
```

### Basic Usage

```cpp
#include <vulkan_stdpar/vulkan_stdpar.hpp>
#include <iostream>

int main() {
    // Drop-in replacement for std::vector
    vulkan_stdpar::unified_vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    
    // Use standard algorithms - automatically GPU accelerated when available
    std::sort(data.begin(), data.end());
    
    // Transform with lambda
    std::transform(data.begin(), data.end(), data.begin(),
                   [](int x) { return x * 2; });
    
    // Reduce/accumulate
    int sum = std::accumulate(data.begin(), data.end(), 0);
    
    std::cout << "Sum: " << sum << std::endl;
    return 0;
}
```

### Compilation

```bash
# With SYCL support
clang++ -std=c++17 -fsycl -I/path/to/include example.cpp -o example

# CPU fallback (no SYCL required)
g++ -std=c++17 -I/path/to/include example.cpp -o example
```

## Examples

See `examples/` directory for complete demonstrations:

- `basic_usage.cpp` - Vector operations and basic algorithms
- `device_selection.cpp` - Device enumeration and selection
- `algorithms_demo.cpp` - Comprehensive algorithm showcase

Build examples:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./examples/basic_usage_example
```

## API Reference

### `unified_vector<T>`

Complete `std::vector<T>` API compatibility:

```cpp
vulkan_stdpar::unified_vector<float> vec(1000);

// All std::vector operations work
vec.push_back(42.0f);
vec.resize(2000);
float val = vec[100];
vec.clear();

// Iterators work with standard algorithms
std::sort(vec.begin(), vec.end());
auto it = std::find(vec.begin(), vec.end(), 42.0f);
```

### Supported Algorithms

- `std::for_each` - Apply function to each element
- `std::transform` - Transform elements
- `std::reduce` / `std::accumulate` - Parallel reduction
- `std::sort` - GPU-optimized sorting
- `std::find`, `std::count` - Search operations

All algorithms automatically use GPU when available, with transparent CPU fallback.

### Device Selection

```cpp
#include <vulkan_stdpar/core/device_selection.hpp>

// Enumerate available devices
auto devices = vulkan_stdpar::device::enumerate_devices();

// Get default device
auto default_dev = vulkan_stdpar::device::get_default_device();
std::cout << "Using: " << default_dev.name << std::endl;

// Rank by performance
auto ranked = vulkan_stdpar::device::rank_devices_by_performance();
```

### Performance Profiling

```cpp
// Enable profiling at compile time
#define VULKAN_STDPAR_ENABLE_PROFILING
#include <vulkan_stdpar/vulkan_stdpar.hpp>

// Profiling is automatic - metrics tracked internally
vulkan_stdpar::profiling::get_statistics();
```

## Architecture

- **Memory Management**: Three-state versioning (clean/host_dirty/device_dirty) with lazy allocation
- **Synchronization**: Automatic sync-on-access with dirty range tracking
- **Thread Safety**: `std::shared_mutex` for concurrent reads, exclusive writes
- **Error Handling**: Exception hierarchy with specific error types

## Performance

CPU fallback performance on M1 Mac (8-core):
- Transform (1M elements): ~100ms
- Sort (1M elements): ~150ms
- Reduce (1M elements): ~50ms

GPU acceleration provides 10-100x speedup on supported hardware.

## Limitations

- SYCL GPU support requires compatible hardware (Intel/AMD/NVIDIA GPUs)
- Apple Silicon (M1/M2) currently runs in CPU fallback mode
- Lambdas must be trivially copyable for GPU execution

## License

MIT License - see LICENSE file for details

## Contributing

Contributions welcome! Please see CONTRIBUTING.md for guidelines.

## Support

- Documentation: See `docs/` directory
- Issues: GitHub issue tracker
- Examples: `examples/` directory

## Version

Current version: 1.0.0 (MVP Release)

## Authors

Vulkan STD-Parallel Team