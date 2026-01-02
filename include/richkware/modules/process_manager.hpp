#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace richkware::modules {

struct ProcessInfo {
    std::uint32_t pid{0};
    std::uint32_t parent_pid{0};
    std::string name;
    std::string executable_path;
    std::uint64_t memory_usage{0}; // KB
    core::TimePoint start_time;
    std::string user;
    bool is_system_process{false};
};

struct ProcessConfig {
    bool include_system_processes{false};
    bool include_memory_info{true};
    std::string user_filter;  // Empty for all users
};

/**
 * @brief Process manager interface for process enumeration and manipulation
 */
class IProcessManager {
public:
    virtual ~IProcessManager() = default;

    /**
     * @brief Get list of running processes
     * @param config Process enumeration configuration
     * @return List of process information or error
     */
    [[nodiscard]] virtual core::Result<std::vector<ProcessInfo>> enumerate_processes(
        const ProcessConfig& config = {}) = 0;

    /**
     * @brief Get information about specific process
     * @param pid Process ID
     * @return Process information or error
     */
    [[nodiscard]] virtual core::Result<ProcessInfo> get_process_info(std::uint32_t pid) = 0;

    /**
     * @brief Terminate process
     * @param pid Process ID to terminate
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> terminate_process(std::uint32_t pid) = 0;

    /**
     * @brief Start new process
     * @param executable_path Path to executable
     * @param arguments Command line arguments
     * @param working_directory Working directory (optional)
     * @return New process ID or error
     */
    [[nodiscard]] virtual core::Result<std::uint32_t> start_process(
        const std::string& executable_path,
        const std::vector<std::string>& arguments = {},
        const std::string& working_directory = "") = 0;

    /**
     * @brief Check if process is running
     * @param pid Process ID
     * @return true if running, false otherwise
     */
    [[nodiscard]] virtual bool is_process_running(std::uint32_t pid) = 0;

    /**
     * @brief Get child processes of a parent
     * @param parent_pid Parent process ID
     * @return List of child process IDs
     */
    [[nodiscard]] virtual core::Result<std::vector<std::uint32_t>> get_child_processes(
        std::uint32_t parent_pid) = 0;

    /**
     * @brief Get current process ID
     * @return Current process ID
     */
    [[nodiscard]] virtual std::uint32_t get_current_pid() const noexcept = 0;

    /**
     * @brief Inject DLL into process (Windows only)
     * @param pid Target process ID
     * @param dll_path Path to DLL file
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> inject_dll(std::uint32_t pid, const std::string& dll_path) = 0;
};

/**
 * @brief Process manager implementation
 */
class ProcessManager : public IProcessManager {
public:
    ProcessManager();
    ~ProcessManager() override;

    // Non-copyable, movable
    ProcessManager(const ProcessManager&) = delete;
    ProcessManager& operator=(const ProcessManager&) = delete;
    ProcessManager(ProcessManager&&) = default;
    ProcessManager& operator=(ProcessManager&&) = default;

    [[nodiscard]] core::Result<std::vector<ProcessInfo>> enumerate_processes(
        const ProcessConfig& config = {}) override;

    [[nodiscard]] core::Result<ProcessInfo> get_process_info(std::uint32_t pid) override;

    [[nodiscard]] core::Result<void> terminate_process(std::uint32_t pid) override;

    [[nodiscard]] core::Result<std::uint32_t> start_process(
        const std::string& executable_path,
        const std::vector<std::string>& arguments = {},
        const std::string& working_directory = "") override;

    [[nodiscard]] bool is_process_running(std::uint32_t pid) override;

    [[nodiscard]] core::Result<std::vector<std::uint32_t>> get_child_processes(
        std::uint32_t parent_pid) override;

    [[nodiscard]] std::uint32_t get_current_pid() const noexcept override;

    [[nodiscard]] core::Result<void> inject_dll(std::uint32_t pid, const std::string& dll_path) override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::modules