#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <optional>
#include <chrono>
#include <vector>
#include <map>

namespace richkware::core {

// Version information
constexpr std::uint32_t VERSION_MAJOR = 2;
constexpr std::uint32_t VERSION_MINOR = 0;
constexpr std::uint32_t VERSION_PATCH = 0;

// Error handling
enum class ErrorCode : std::uint32_t {
    Success = 0,
    NetworkError,
    CryptoError,
    SystemError,
    ConfigError,
    AuthenticationError,
    PermissionDenied,
    InvalidArgument,
    ResourceUnavailable
};

class RichkwareError : public std::exception {
public:
    explicit RichkwareError(ErrorCode code, std::string message = "")
        : code_(code), message_(std::move(message)) {}
    
    [[nodiscard]] const char* what() const noexcept override {
        return message_.c_str();
    }
    
    [[nodiscard]] ErrorCode code() const noexcept { return code_; }

private:
    ErrorCode code_;
    std::string message_;
};

// Simple Result type using optional
template<typename T>
class Result {
public:
    Result(T value) : value_(std::move(value)), has_error_(false) {}
    Result(RichkwareError error) : error_(std::move(error)), has_error_(true) {}
    
    operator bool() const { return !has_error_; }
    const T& value() const { return value_.value(); }
    T& value() { return value_.value(); }
    const RichkwareError& error() const { return error_; }
    
private:
    std::optional<T> value_;
    RichkwareError error_{ErrorCode::Success};
    bool has_error_;
};

// Specialization for void
template<>
class Result<void> {
public:
    Result() : has_error_(false) {}
    Result(RichkwareError error) : error_(std::move(error)), has_error_(true) {}
    
    operator bool() const { return !has_error_; }
    const RichkwareError& error() const { return error_; }
    
private:
    RichkwareError error_{ErrorCode::Success};
    bool has_error_;
};

// Common types
using Bytes = std::vector<std::uint8_t>;
using Duration = std::chrono::milliseconds;
using TimePoint = std::chrono::steady_clock::time_point;

// Configuration structure
struct Config {
    std::string app_name{"Richkware"};
    std::string encryption_key;
    std::string server_address{"127.0.0.1"};
    std::uint16_t server_port{8080};
    std::string user_id;
    Duration connection_timeout{30000};
    bool enable_encryption{true};
    bool enable_stealth{true};
    std::string log_level{"info"};
};

// Forward declarations
class Agent;
class NetworkManager;
class CryptoManager;
class SystemManager;

} // namespace richkware::core