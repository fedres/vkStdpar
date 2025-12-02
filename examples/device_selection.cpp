/**
 * @file device_selection_example.cpp
 * @brief Example demonstrating device enumeration and selection
 */

#include <vulkan_stdpar/vulkan_stdpar.hpp>
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "Vulkan STD-Parallel - Device Selection Example\n";
    std::cout << "=============================================\n\n";
    
    try {
        // Enumerate all devices
        std::cout << "📋 Available Devices:\n";
        std::cout << std::string(50, '-') << "\n";
        
        auto devices = vulkan_stdpar::device::enumerate_devices();
        
        if (devices.empty()) {
            std::cout << "  No devices found.\n";
        } else {
            for (size_t i = 0; i < devices.size(); ++i) {
                const auto& dev = devices[i];
                std::cout << "  Device " << i << ": " << dev.name << "\n";
                std::cout << "    Vendor: " << dev.vendor << "\n";
                std::cout << "    Memory: " << (dev.memory_size / (1024 * 1024)) << " MB\n";
                std::cout << "    Compute Units: " << dev.max_compute_units << "\n";
                std::cout << "    Status: " << vulkan_stdpar::device::get_device_status(dev) << "\n";
                std::cout << "\n";
            }
        }
        
        // Get default device
        std::cout << "\n🎯 Default Device:\n";
        std::cout << std::string(50, '-') << "\n";
        
        auto default_dev = vulkan_stdpar::device::get_default_device();
        std::cout << "  Name: " << default_dev.name << "\n";
        std::cout << "  Vendor: " << default_dev.vendor << "\n";
        std::cout << "  Performance Score: " << std::fixed << std::setprecision(2) 
                  << default_dev.performance_score() << "\n";
        
        // Rank devices by performance
        std::cout << "\n⚡ Devices Ranked by Performance:\n";
        std::cout << std::string(50, '-') << "\n";
        
        auto ranked = vulkan_stdpar::device::rank_devices_by_performance();
        for (size_t i = 0; i < ranked.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << ranked[i].name 
                      << " (score: " << std::fixed << std::setprecision(2)
                      << ranked[i].performance_score() << ")\n";
        }
        
        std::cout << "\n✅ Device selection example completed successfully!\n";
        
    } catch (const vulkan_stdpar::vulkan_stdpar_exception& e) {
        std::cerr << "❌ Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
