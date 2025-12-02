/**
 * @file exceptions.hpp
 * @brief Exception hierarchy for Vulkan STD-Parallel library
 * @author Vulkan STD-Parallel Team
 * @version 1.0
 * @date 2025-12-02
 * 
 * This file defines the exception hierarchy and error handling utilities
 * for the Vulkan STD-Parallel library.
 */

#ifndef VULKAN_STDPAR_CORE_EXCEPTIONS_HPP
#define VULKAN_STDPAR_CORE_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>
#include <sstream>

namespace vulkan_stdpar {

/**
 * @brief Base exception class for all Vulkan STD-Parallel exceptions
 */
class vulkan_stdpar_exception : public std::runtime_error {
public:
    explicit vulkan_stdpar_exception(const std::string& message)
        : std::runtime_error(message)
    {}
    
    explicit vulkan_stdpar_exception(const char* message)
        : std::runtime_error(message)
    {}
};

/**
 * @brief Exception thrown when host/device synchronization fails
 */
class synchronization_exception : public vulkan_stdpar_exception {
public:
    explicit synchronization_exception(const std::string& message)
        : vulkan_stdpar_exception("Synchronization error: " + message)
    {}
};

/**
 * @brief Exception thrown when kernel compilation fails
 */
class compilation_exception : public vulkan_stdpar_exception {
public:
    explicit compilation_exception(const std::string& message)
        : vulkan_stdpar_exception("Compilation error: " + message)
    {}
};

/**
 * @brief Exception thrown when GPU memory is exhausted
 */
class out_of_memory_exception : public vulkan_stdpar_exception {
public:
    explicit out_of_memory_exception(size_t requested_bytes)
        : vulkan_stdpar_exception(format_message(requested_bytes))
        , requested_bytes_(requested_bytes)
    {}
    
    size_t requested_bytes() const { return requested_bytes_; }
    
private:
    size_t requested_bytes_;
    
    static std::string format_message(size_t bytes) {
        std::ostringstream oss;
        oss << "Out of GPU memory: requested " 
            << (bytes / (1024.0 * 1024.0)) << " MB";
        return oss.str();
    }
};

/**
 * @brief Exception thrown when GPU device is lost or removed
 */
class device_lost_exception : public vulkan_stdpar_exception {
public:
    explicit device_lost_exception(const std::string& device_name = "")
        : vulkan_stdpar_exception("Device lost: " + 
            (device_name.empty() ? "unknown device" : device_name))
        , device_name_(device_name)
    {}
    
    const std::string& device_name() const { return device_name_; }
    
private:
    std::string device_name_;
};

/**
 * @brief Exception thrown when an unsupported operation is attempted
 */
class unsupported_operation_exception : public vulkan_stdpar_exception {
public:
    explicit unsupported_operation_exception(const std::string& operation)
        : vulkan_stdpar_exception("Unsupported operation: " + operation)
        , operation_(operation)
    {}
    
    const std::string& operation() const { return operation_; }
    
private:
    std::string operation_;
};

/**
 * @brief Exception thrown when device is not found
 */
class device_not_found_exception : public vulkan_stdpar_exception {
public:
    explicit device_not_found_exception(const std::string& criteria)
        : vulkan_stdpar_exception("Device not found: " + criteria)
        , criteria_(criteria)
    {}
    
    const std::string& criteria() const { return criteria_; }
    
private:
    std::string criteria_;
};

/**
 * @brief Exception thrown when device is unavailable
 */
class device_unavailable_exception : public vulkan_stdpar_exception {
public:
    explicit device_unavailable_exception(const std::string& reason)
        : vulkan_stdpar_exception("Device unavailable: " + reason)
        , reason_(reason)
    {}
    
    const std::string& reason() const { return reason_; }
    
private:
    std::string reason_;
};

/**
 * @brief Exception thrown when device initialization fails
 */
class device_initialization_exception : public vulkan_stdpar_exception {
public:
    explicit device_initialization_exception(const std::string& reason)
        : vulkan_stdpar_exception("Device initialization failed: " + reason)
        , reason_(reason)
    {}
    
    const std::string& reason() const { return reason_; }
    
private:
    std::string reason_;
};

/**
 * @brief Exception thrown when queue creation fails
 */
class queue_creation_exception : public vulkan_stdpar_exception {
public:
    explicit queue_creation_exception(const std::string& reason)
        : vulkan_stdpar_exception("Queue creation failed: " + reason)
        , reason_(reason)
    {}
    
    const std::string& reason() const { return reason_; }
    
private:
    std::string reason_;
};

/**
 * @brief Exception thrown when invalid argument is passed
 */
class invalid_argument_exception : public vulkan_stdpar_exception {
public:
    explicit invalid_argument_exception(const std::string& argument, const std::string& reason)
        : vulkan_stdpar_exception("Invalid argument '" + argument + "': " + reason)
        , argument_(argument)
        , reason_(reason)
    {}
    
    const std::string& argument() const { return argument_; }
    const std::string& reason() const { return reason_; }
    
private:
    std::string argument_;
    std::string reason_;
};

/**
 * @brief Error handling macro for try-catch blocks
 * 
 * Usage:
 *   VULKAN_STDPAR_TRY {
 *       // code that may throw
 *   }
 *   VULKAN_STD PAR_CATCH(error) {
 *       // handle error
 *   }
 */
#define VULKAN_STDPAR_TRY try

#define VULKAN_STDPAR_CATCH(exception_var) \
    catch (const vulkan_stdpar::vulkan_stdpar_exception& exception_var)

#define VULKAN_STDPAR_CATCH_ALL catch (...)

/**
 * @brief Assert macro that throws exception instead of aborting
 */
#ifdef VULKAN_STDPAR_DEBUG
#define VULKAN_STDPAR_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            throw vulkan_stdpar::vulkan_stdpar_exception( \
                std::string("Assertion failed: ") + #condition + " - " + message); \
        } \
    } while (0)
#else
#define VULKAN_STDPAR_ASSERT(condition, message) ((void)0)
#endif

/**
 * @brief Throw exception with formatted message
 */
#define VULKAN_STDPAR_THROW(exception_type, ...) \
    throw vulkan_stdpar::exception_type(__VA_ARGS__)

} // namespace vulkan_stdpar

#endif // VULKAN_STDPAR_CORE_EXCEPTIONS_HPP
