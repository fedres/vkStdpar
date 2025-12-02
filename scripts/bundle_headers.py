#!/usr/bin/env python3
"""
Header bundler script for Vulkan STD-Parallel library
Concatenates all headers into a single vulkan_stdpar.hpp file
"""

import os
import re
from pathlib import Path

def extract_includes(file_path):
    """Extract local includes from a header file"""
    includes = []
    with open(file_path, 'r') as f:
        for line in f:
            # Match local includes: #include "..."
            match = re.match(r'#include\s+"([^"]+)"', line)
            if match:
                includes.append(match.group(1))
    return includes

def read_header(file_path, processed=None):
    """Read header file and resolve dependencies"""
    if processed is None:
        processed = set()
    
    if file_path in processed:
        return ""
    
    processed.add(file_path)
    
    content_lines = []
    with open(file_path, 'r') as f:
        in_header_guard = False
        for line in f:
            # Skip include guards
            if '#ifndef' in line or '#define' in line:
                if any(guard in line for guard in ['_HPP', '_H']):
                    in_header_guard = True
                    continue
            if in_header_guard and '#endif' in line:
                in_header_guard = False
                continue
            
            # Skip local includes (we'll resolve dependencies)
            if re.match(r'#include\s+"', line):
                continue
            
            content_lines.append(line)
    
    return ''.join(content_lines)

def bundle_headers(include_dir, output_file):
    """Bundle all headers into a single file"""
    include_path = Path(include_dir)
    
    # Define dependency order
    header_order = [
        # Core infrastructure first
        'core/exceptions.hpp',
        'core/profiling.hpp',
        'core/versioning_engine.hpp',
        'core/device_selection.hpp',
        # Containers
        'containers/unified_reference.hpp',
        'containers/unified_vector.hpp',
        # Iterators
        'iterators/unified_iterator.hpp',
        # Algorithms
        'algorithms/parallel_invoker.hpp',
    ]
    
    output_content = []
    
    # Add header comment
    output_content.append("""/**
 * @file vulkan_stdpar.hpp
 * @brief Single-header bundle for Vulkan STD-Parallel library
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This is an automatically generated single-header version of the
 * Vulkan STD-Parallel library. Include this file to get access to
 * all GPU-accelerated standard parallel algorithms and unified_vector.
 * 
 * Usage:
 *   #include <vulkan_stdpar.hpp>
 *   
 *   vulkan_stdpar::unified_vector<int> data = {1, 2, 3, 4, 5};
 *   std::sort(data.begin(), data.end());
 */

#ifndef VULKAN_STDPAR_VULKAN_STDPAR_HPP
#define VULKAN_STDPAR_VULKAN_STDPAR_HPP

// Standard library includes
#include <atomic>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <functional>
#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <string>
#include <sstream>
#include <chrono>
#include <iterator>
#include <initializer_list>
#include <utility>
#include <limits>

// SYCL includes (optional)
#ifdef VULKAN_STDPAR_USE_SYCL
#include <sycl/sycl.hpp>
#endif

// Vulkan includes (optional)
#ifdef VULKAN_STDPAR_USE_VULKAN
#include <vulkan/vulkan.h>
#endif

""")
    
    # Process each header in order
    processed = set()
    for header_rel_path in header_order:
        header_path = include_path / 'vulkan_stdpar' / header_rel_path
        if header_path.exists():
            print(f"Processing {header_rel_path}...")
            content = read_header(header_path, processed)
            if content.strip():
                output_content.append(f"\n// ========== {header_rel_path} ==========\n\n")
                output_content.append(content)
    
    # Add footer
    output_content.append("\n#endif // VULKAN_STDPAR_VULKAN_STDPAR_HPP\n")
    
    # Write output
    with open(output_file, 'w') as f:
        f.write(''.join(output_content))
    
    print(f"\nBundled headers written to: {output_file}")
    print(f"Total size: {os.path.getsize(output_file)} bytes")

if __name__ == '__main__':
    import sys
    
    # Default paths
    include_dir = Path(__file__).parent.parent / 'include'
    output_file = include_dir / 'vulkan_stdpar.hpp'
    
    if len(sys.argv) > 1:
        include_dir = Path(sys.argv[1])
    if len(sys.argv) > 2:
        output_file = Path(sys.argv[2])
    
    print(f"Include directory: {include_dir}")
    print(f"Output file: {output_file}")
    print()
    
    bundle_headers(include_dir, output_file)
