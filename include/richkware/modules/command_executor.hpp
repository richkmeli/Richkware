#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <functional>
#include <future>
#include <chrono>

namespace richkware::modules {

/**
 * @brief Command execution result
 */
struct ExecutionResult {
    int exit_code{0};
    std::string stdout_output;
    std::string stderr_output;
    core::Duration execution_time{0};
    bool timed_out{false};
};

/**
 * @brief Command execution options
 */
struct ExecutionOptions {
    core::Duration timeout{30000};
    bool capture_output{true};
    bool run_hidden{true};
    std::string working_directory;
    std::map<std::string, std::string> environment_variables;
};

/**
 * @brief Command executor interface
 */
class ICommandExecutor {
public:
    virtual ~ICommandExecutor() = default;
    
    /**
     * @brief Execute command synchronously
     * @param command Command to execute
     * @param options Execution options
     * @return Execution result or error
     */
    [[nodiscard]] virtual core::Result<ExecutionResult> execute(
        const std::string& command,
        const ExecutionOptions& options = {}) = 0;
    
    /**
     * @brief Execute command asynchronously
     * @param command Command to execute
     * @param options Execution options
     * @return Future with execution result or error
     */
    [[nodiscard]] virtual std::future<core::Result<ExecutionResult>> execute_async(
        const std::string& command,
        const ExecutionOptions& options = {}) = 0;
    
    /**
     * @brief Kill running process
     * @param process_id Process ID to kill
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> kill_process(std::uint32_t process_id) = 0;
};

/**
 * @brief Windows command executor implementation
 */
class WindowsCommandExecutor : public ICommandExecutor {
public:
    WindowsCommandExecutor();
    ~WindowsCommandExecutor() override;
    
    // Non-copyable, movable
    WindowsCommandExecutor(const WindowsCommandExecutor&) = delete;
    WindowsCommandExecutor& operator=(const WindowsCommandExecutor&) = delete;
    WindowsCommandExecutor(WindowsCommandExecutor&&) = default;
    WindowsCommandExecutor& operator=(WindowsCommandExecutor&&) = default;
    
    [[nodiscard]] core::Result<ExecutionResult> execute(
        const std::string& command,
        const ExecutionOptions& options = {}) override;
    
    [[nodiscard]] std::future<core::Result<ExecutionResult>> execute_async(
        const std::string& command,
        const ExecutionOptions& options = {}) override;
    
    [[nodiscard]] core::Result<void> kill_process(std::uint32_t process_id) override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief PowerShell command executor
 */
class PowerShellExecutor : public ICommandExecutor {
public:
    PowerShellExecutor();
    ~PowerShellExecutor() override;
    
    // Non-copyable, movable
    PowerShellExecutor(const PowerShellExecutor&) = delete;
    PowerShellExecutor& operator=(const PowerShellExecutor&) = delete;
    PowerShellExecutor(PowerShellExecutor&&) = default;
    PowerShellExecutor& operator=(PowerShellExecutor&&) = default;
    
    [[nodiscard]] core::Result<ExecutionResult> execute(
        const std::string& command,
        const ExecutionOptions& options = {}) override;
    
    [[nodiscard]] std::future<core::Result<ExecutionResult>> execute_async(
        const std::string& command,
        const ExecutionOptions& options = {}) override;
    
    [[nodiscard]] core::Result<void> kill_process(std::uint32_t process_id) override;
    
    /**
     * @brief Execute PowerShell script from file
     * @param script_path Path to PowerShell script
     * @param options Execution options
     * @return Execution result or error
     */
    [[nodiscard]] core::Result<ExecutionResult> execute_script(
        const std::string& script_path,
        const ExecutionOptions& options = {});

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief High-level command executor with multiple backends
 */
class CommandExecutor {
public:
    /**
     * @brief Supported executor types
     */
    enum class ExecutorType {
        CMD,
        PowerShell,
        WMI
    };
    
    /**
     * @brief Construct with default executor type
     * @param default_type Default executor to use
     */
    explicit CommandExecutor(ExecutorType default_type = ExecutorType::CMD);
    
    ~CommandExecutor();
    
    // Non-copyable, movable
    CommandExecutor(const CommandExecutor&) = delete;
    CommandExecutor& operator=(const CommandExecutor&) = delete;
    CommandExecutor(CommandExecutor&&) = default;
    CommandExecutor& operator=(CommandExecutor&&) = default;
    
    /**
     * @brief Execute command with default executor
     * @param command Command to execute
     * @param options Execution options
     * @return Execution result or error
     */
    [[nodiscard]] core::Result<ExecutionResult> execute(
        const std::string& command,
        const ExecutionOptions& options = {});
    
    /**
     * @brief Execute command with specific executor
     * @param command Command to execute
     * @param executor_type Executor type to use
     * @param options Execution options
     * @return Execution result or error
     */
    [[nodiscard]] core::Result<ExecutionResult> execute_with(
        const std::string& command,
        ExecutorType executor_type,
        const ExecutionOptions& options = {});
    
    /**
     * @brief Execute command asynchronously
     * @param command Command to execute
     * @param options Execution options
     * @return Future with execution result or error
     */
    [[nodiscard]] std::future<core::Result<ExecutionResult>> execute_async(
        const std::string& command,
        const ExecutionOptions& options = {});
    
    /**
     * @brief Set command execution callback
     * @param callback Callback function called before command execution
     */
    void set_execution_callback(
        std::function<void(const std::string&, ExecutorType)> callback);
    
    /**
     * @brief Get available executor types
     * @return Vector of available executor types
     */
    [[nodiscard]] std::vector<ExecutorType> available_executors() const;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::modules