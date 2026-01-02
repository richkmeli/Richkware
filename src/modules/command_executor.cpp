#include "richkware/modules/command_executor.hpp"
#include <iostream>
#include <sstream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#endif

namespace richkware::modules {

// WindowsCommandExecutor implementation
class WindowsCommandExecutor::Impl {
public:
    core::Result<ExecutionResult> execute_command(const std::string& command, 
                                                 const ExecutionOptions& options) {
        auto start_time = std::chrono::steady_clock::now();
        
#ifdef _WIN32
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
        
        HANDLE stdout_read, stdout_write;
        HANDLE stderr_read, stderr_write;
        
        if (!CreatePipe(&stdout_read, &stdout_write, &sa, 0) ||
            !CreatePipe(&stderr_read, &stderr_write, &sa, 0)) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to create pipes"};
        }
        
        SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0);
        
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.hStdError = stderr_write;
        si.hStdOutput = stdout_write;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.dwFlags |= STARTF_USESTDHANDLES;
        
        if (options.run_hidden) {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
        }
        
        std::string cmd_line = "cmd.exe /c " + command;
        
        if (!CreateProcess(NULL, const_cast<char*>(cmd_line.c_str()), NULL, NULL, TRUE, 
                          CREATE_NO_WINDOW, NULL, 
                          options.working_directory.empty() ? NULL : options.working_directory.c_str(),
                          &si, &pi)) {
            CloseHandle(stdout_read);
            CloseHandle(stdout_write);
            CloseHandle(stderr_read);
            CloseHandle(stderr_write);
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to create process"};
        }
        
        CloseHandle(stdout_write);
        CloseHandle(stderr_write);
        
        // Wait for process with timeout
        DWORD wait_result = WaitForSingleObject(pi.hProcess, static_cast<DWORD>(options.timeout.count()));
        
        ExecutionResult result;
        result.timed_out = (wait_result == WAIT_TIMEOUT);
        
        if (result.timed_out) {
            TerminateProcess(pi.hProcess, 1);
            result.exit_code = 1;
        } else {
            DWORD exit_code;
            GetExitCodeProcess(pi.hProcess, &exit_code);
            result.exit_code = static_cast<int>(exit_code);
        }
        
        // Read output
        if (options.capture_output) {
            char buffer[4096];
            DWORD bytes_read;
            
            while (ReadFile(stdout_read, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {
                buffer[bytes_read] = '\0';
                result.stdout_output += buffer;
            }
            
            while (ReadFile(stderr_read, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {
                buffer[bytes_read] = '\0';
                result.stderr_output += buffer;
            }
        }
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(stdout_read);
        CloseHandle(stderr_read);
        
#else
        // Linux implementation using popen for simplicity
        ExecutionResult result;
        std::string full_command = command;
        if (!options.working_directory.empty()) {
            full_command = "cd " + options.working_directory + " && " + command;
        }
        
        FILE* pipe = popen(full_command.c_str(), "r");
        if (!pipe) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to execute command"};
        }
        
        if (options.capture_output) {
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result.stdout_output += buffer;
            }
        }
        
        result.exit_code = pclose(pipe);
#endif
        
        auto end_time = std::chrono::steady_clock::now();
        result.execution_time = std::chrono::duration_cast<core::Duration>(end_time - start_time);
        
        return result;
    }
};

WindowsCommandExecutor::WindowsCommandExecutor() : pimpl_(std::make_unique<Impl>()) {}
WindowsCommandExecutor::~WindowsCommandExecutor() = default;

core::Result<ExecutionResult> WindowsCommandExecutor::execute(const std::string& command,
                                                             const ExecutionOptions& options) {
    return pimpl_->execute_command(command, options);
}

std::future<core::Result<ExecutionResult>> WindowsCommandExecutor::execute_async(
    const std::string& command, const ExecutionOptions& options) {
    return std::async(std::launch::async, [this, command, options]() {
        return execute(command, options);
    });
}

core::Result<void> WindowsCommandExecutor::kill_process(std::uint32_t process_id) {
#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, process_id);
    if (!process) {
        return core::RichkwareError{core::ErrorCode::SystemError, "Failed to open process"};
    }
    
    if (!TerminateProcess(process, 1)) {
        CloseHandle(process);
        return core::RichkwareError{core::ErrorCode::SystemError, "Failed to terminate process"};
    }
    
    CloseHandle(process);
    return core::Result<void>{};
#else
    if (kill(process_id, SIGTERM) == -1) {
        return core::RichkwareError{core::ErrorCode::SystemError, "Failed to kill process"};
    }
    return core::Result<void>{};
#endif
}

// PowerShellExecutor implementation
class PowerShellExecutor::Impl {
public:
    std::unique_ptr<WindowsCommandExecutor> executor_;

    Impl() : executor_(std::make_unique<WindowsCommandExecutor>()) {}

    core::Result<ExecutionResult> execute_command(const std::string& command,
                                                 const ExecutionOptions& options) {
        return executor_->execute(command, options);
    }
};

PowerShellExecutor::PowerShellExecutor() : pimpl_(std::make_unique<Impl>()) {}
PowerShellExecutor::~PowerShellExecutor() = default;

core::Result<ExecutionResult> PowerShellExecutor::execute(const std::string& command,
                                                         const ExecutionOptions& options) {
#ifdef _WIN32
    std::string ps_command = "powershell.exe -Command \"" + command + "\"";
#else
    std::string ps_command = "pwsh -Command \"" + command + "\"";
#endif
    return pimpl_->execute_command(ps_command, options);
}

std::future<core::Result<ExecutionResult>> PowerShellExecutor::execute_async(
    const std::string& command, const ExecutionOptions& options) {
    return std::async(std::launch::async, [this, command, options]() {
        return execute(command, options);
    });
}

core::Result<void> PowerShellExecutor::kill_process(std::uint32_t process_id) {
    WindowsCommandExecutor executor;
    return executor.kill_process(process_id);
}

core::Result<ExecutionResult> PowerShellExecutor::execute_script(const std::string& script_path,
                                                                const ExecutionOptions& options) {
#ifdef _WIN32
    std::string ps_command = "powershell.exe -ExecutionPolicy Bypass -File \"" + script_path + "\"";
#else
    std::string ps_command = "pwsh -ExecutionPolicy Bypass -File \"" + script_path + "\"";
#endif
    return pimpl_->execute_command(ps_command, options);
}

// CommandExecutor implementation
class CommandExecutor::Impl {
public:
    ExecutorType default_type_;
    std::unique_ptr<WindowsCommandExecutor> cmd_executor_;
    std::unique_ptr<PowerShellExecutor> ps_executor_;
    std::function<void(const std::string&, ExecutorType)> callback_;
    
    Impl(ExecutorType type) : default_type_(type) {
        cmd_executor_ = std::make_unique<WindowsCommandExecutor>();
        ps_executor_ = std::make_unique<PowerShellExecutor>();
    }
    
    ICommandExecutor* get_executor(ExecutorType type) {
        switch (type) {
            case ExecutorType::CMD:
                return cmd_executor_.get();
            case ExecutorType::PowerShell:
                return ps_executor_.get();
            case ExecutorType::WMI:
                // WMI not implemented, fallback to CMD
                return cmd_executor_.get();
            default:
                return cmd_executor_.get();
        }
    }
};

CommandExecutor::CommandExecutor(ExecutorType default_type) 
    : pimpl_(std::make_unique<Impl>(default_type)) {}

CommandExecutor::~CommandExecutor() = default;

core::Result<ExecutionResult> CommandExecutor::execute(const std::string& command,
                                                      const ExecutionOptions& options) {
    return execute_with(command, pimpl_->default_type_, options);
}

core::Result<ExecutionResult> CommandExecutor::execute_with(const std::string& command,
                                                           ExecutorType executor_type,
                                                           const ExecutionOptions& options) {
    if (pimpl_->callback_) {
        pimpl_->callback_(command, executor_type);
    }
    
    ICommandExecutor* executor = pimpl_->get_executor(executor_type);
    return executor->execute(command, options);
}

std::future<core::Result<ExecutionResult>> CommandExecutor::execute_async(
    const std::string& command, const ExecutionOptions& options) {
    return std::async(std::launch::async, [this, command, options]() {
        return execute(command, options);
    });
}

void CommandExecutor::set_execution_callback(
    std::function<void(const std::string&, ExecutorType)> callback) {
    pimpl_->callback_ = std::move(callback);
}

std::vector<CommandExecutor::ExecutorType> CommandExecutor::available_executors() const {
    std::vector<ExecutorType> executors = {ExecutorType::CMD};
    
#ifdef _WIN32
    executors.push_back(ExecutorType::PowerShell);
    executors.push_back(ExecutorType::WMI);
#else
    // Check if PowerShell Core is available
    if (system("which pwsh > /dev/null 2>&1") == 0) {
        executors.push_back(ExecutorType::PowerShell);
    }
#endif
    
    return executors;
}

} // namespace richkware::modules