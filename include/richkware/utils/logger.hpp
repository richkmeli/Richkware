#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <mutex>
#include <iomanip>
#include <ctime>

namespace richkware::utils {

inline std::tm* safeLocalTime(const std::time_t* timep, std::tm& out) {
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) || defined(__unix__) || defined(__APPLE__)
    if (localtime_r(timep, &out) == nullptr) {
        return nullptr;
    }
    return &out;
#else
    (void)out;
    return std::localtime(timep);
#endif
}

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLevel(LogLevel level) {
        current_level_ = level;
    }

    void setOutput(std::ostream& stream) {
        output_stream_ = &stream;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < current_level_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;

        // Timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf{};
        std::tm* tm_ptr = nullptr;

#if defined(_WIN32) || defined(_WIN64)
        // Use thread-safe localtime_s on Windows
        if (::localtime_s(&tm_buf, &time_t) == 0) {
            tm_ptr = &tm_buf;
        }
#elif defined(__unix__) || defined(__APPLE__)
        // Use thread-safe localtime_r on POSIX
        if (::localtime_r(&time_t, &tm_buf) != nullptr) {
            tm_ptr = &tm_buf;
        }
#else
        // Fallback to potentially non-thread-safe std::localtime
        tm_ptr = std::localtime(&time_t);
#endif
        if (tm_ptr != nullptr) {
            oss << std::put_time(tm_ptr, "%a %b %d %H:%M:%S %Y") << " ";
        } else {
            oss << "unknown-time ";
        }

        // Level
        switch (level) {
            case LogLevel::DEBUG: oss << "[DEBUG] "; break;
            case LogLevel::INFO: oss << "[INFO] "; break;
            case LogLevel::WARNING: oss << "[WARNING] "; break;
            case LogLevel::ERROR: oss << "[ERROR] "; break;
            case LogLevel::CRITICAL: oss << "[CRITICAL] "; break;
        }

        oss << message << std::endl;

        *output_stream_ << oss.str();
        output_stream_->flush();
    }

    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args&&... args) {
        if (level < current_level_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream oss;

        // Timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf{};
        std::tm* tm_ptr = safeLocalTime(&time_t, tm_buf);
        if (tm_ptr != nullptr) {
            oss << std::put_time(tm_ptr, "%a %b %d %H:%M:%S %Y") << " ";
        } else {
            oss << "unknown-time ";
        }

        // Level
        switch (level) {
            case LogLevel::DEBUG: oss << "[DEBUG] "; break;
            case LogLevel::INFO: oss << "[INFO] "; break;
            case LogLevel::WARNING: oss << "[WARNING] "; break;
            case LogLevel::ERROR: oss << "[ERROR] "; break;
            case LogLevel::CRITICAL: oss << "[CRITICAL] "; break;
        }

        // Format message
        formatMessage(oss, format, std::forward<Args>(args)...);
        oss << std::endl;

        *output_stream_ << oss.str();
        output_stream_->flush();
    }

    template<typename... Args>
    void debug(const std::string& format, Args&&... args) {
        log(LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const std::string& format, Args&&... args) {
        log(LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(const std::string& format, Args&&... args) {
        log(LogLevel::WARNING, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const std::string& format, Args&&... args) {
        log(LogLevel::ERROR, format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(const std::string& format, Args&&... args) {
        log(LogLevel::CRITICAL, format, std::forward<Args>(args)...);
    }

private:
    Logger() : current_level_(LogLevel::INFO), output_stream_(&std::cout) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel current_level_;
    std::ostream* output_stream_;
    std::mutex mutex_;

    template<typename T>
    void formatMessage(std::ostringstream& oss, const std::string& format, T&& arg) {
        size_t pos = format.find("{}");
        if (pos != std::string::npos) {
            oss << format.substr(0, pos) << arg << format.substr(pos + 2);
        } else {
            oss << format << " " << arg;
        }
    }

    template<typename T, typename... Args>
    void formatMessage(std::ostringstream& oss, const std::string& format, T&& arg, Args&&... args) {
        size_t pos = format.find("{}");
        if (pos != std::string::npos) {
            oss << format.substr(0, pos) << arg;
            formatMessage(oss, format.substr(pos + 2), std::forward<Args>(args)...);
        } else {
            oss << format << " " << arg;
            formatMessage(oss, "", std::forward<Args>(args)...);
        }
    }
};

// Convenience macros
#define LOG_DEBUG(...) richkware::utils::Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) richkware::utils::Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARNING(...) richkware::utils::Logger::getInstance().warning(__VA_ARGS__)
#define LOG_ERROR(...) richkware::utils::Logger::getInstance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) richkware::utils::Logger::getInstance().critical(__VA_ARGS__)

} // namespace richkware::utils