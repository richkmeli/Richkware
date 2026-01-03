#include "richkware/modules/process_manager.hpp"
#include "richkware/utils/logger.hpp"
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#endif

namespace richkware::modules {

// ProcessManager implementation
class ProcessManager::Impl {
public:
    core::Result<std::vector<ProcessInfo>> enumerate_processes(const ProcessConfig& config) {
#ifdef _WIN32
        return enumerate_processes_windows(config);
#elif defined(__linux__)
        return enumerate_processes_linux(config);
#else
        (void)config;
        return core::RichkwareError{core::ErrorCode::SystemError, "Process enumeration not supported on this platform"};
#endif
    }

    core::Result<ProcessInfo> get_process_info(std::uint32_t pid) {
#ifdef _WIN32
        return get_process_info_windows(pid);
#elif defined(__linux__)
        return get_process_info_linux(pid);
#else
        (void)pid;
        return core::RichkwareError{core::ErrorCode::SystemError, "Process info not supported on this platform"};
#endif
    }

    core::Result<void> terminate_process(std::uint32_t pid) {
#ifdef _WIN32
        return terminate_process_windows(pid);
#elif defined(__linux__)
        return terminate_process_linux(pid);
#else
        (void)pid;
        return core::RichkwareError{core::ErrorCode::SystemError, "Process termination not supported on this platform"};
#endif
    }

    core::Result<std::uint32_t> start_process(const std::string& executable_path,
                                            const std::vector<std::string>& arguments,
                                            const std::string& working_directory) {
#ifdef _WIN32
        return start_process_windows(executable_path, arguments, working_directory);
#elif defined(__linux__)
        return start_process_linux(executable_path, arguments, working_directory);
#else
        (void)executable_path;
        (void)arguments;
        (void)working_directory;
        return core::RichkwareError{core::ErrorCode::SystemError, "Process starting not supported on this platform"};
#endif
    }

    bool is_process_running(std::uint32_t pid) {
        auto result = get_process_info(pid);
        return static_cast<bool>(result);
    }

    core::Result<std::vector<std::uint32_t>> get_child_processes(std::uint32_t parent_pid) {
        auto all_processes = enumerate_processes(ProcessConfig{});
        if (!all_processes) {
            return all_processes.error();
        }

        std::vector<std::uint32_t> children;
        for (const auto& proc : all_processes.value()) {
            if (proc.parent_pid == parent_pid) {
                children.push_back(proc.pid);
            }
        }

        return children;
    }

    std::uint32_t get_current_pid() const noexcept {
#ifdef _WIN32
        return GetCurrentProcessId();
#else
        return getpid();
#endif
    }

    core::Result<void> inject_dll(std::uint32_t pid, const std::string& dll_path) {
#ifdef _WIN32
        return inject_dll_windows(pid, dll_path);
#else
        (void)pid;
        (void)dll_path;
        return core::RichkwareError{core::ErrorCode::SystemError, "DLL injection not supported on Linux"};
#endif
    }

private:
#ifdef _WIN32
    core::Result<std::vector<ProcessInfo>> enumerate_processes_windows(const ProcessConfig& config) {
        std::vector<ProcessInfo> processes;

        HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot create process snapshot"};
        }

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(hProcessSnap, &pe32)) {
            CloseHandle(hProcessSnap);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot enumerate processes"};
        }

        do {
            ProcessInfo info;
            info.pid = pe32.th32ProcessID;
            info.parent_pid = pe32.th32ParentProcessID;

            // Convert TCHAR to string
#ifdef UNICODE
            std::wstring wname(pe32.szExeFile);
            info.name = std::string(wname.begin(), wname.end());
#else
            info.name = pe32.szExeFile;
#endif

            // Get executable path and memory info if requested
            if (config.include_memory_info) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
                if (hProcess) {
                    // Get executable path
                    TCHAR exePath[MAX_PATH];
                    if (GetModuleFileNameEx(hProcess, NULL, exePath, MAX_PATH)) {
#ifdef UNICODE
                        std::wstring wpath(exePath);
                        info.executable_path = std::string(wpath.begin(), wpath.end());
#else
                        info.executable_path = exePath;
#endif
                    }

                    // Get memory usage
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                        info.memory_usage = pmc.WorkingSetSize / 1024; // Convert to KB
                    }

                    CloseHandle(hProcess);
                }
            }

            // Skip system processes if not requested
            if (!config.include_system_processes && is_system_process_windows(info.pid)) {
                continue;
            }

            // Apply user filter if specified
            if (!config.user_filter.empty()) {
                // Simplified - in real implementation check process user
                continue;
            }

            processes.push_back(info);
        } while (Process32Next(hProcessSnap, &pe32));

        CloseHandle(hProcessSnap);
        return processes;
    }

    core::Result<ProcessInfo> get_process_info_windows(std::uint32_t pid) {
        auto all_processes = enumerate_processes(ProcessConfig{});
        if (!all_processes) {
            return all_processes.error();
        }

        for (const auto& proc : all_processes.value()) {
            if (proc.pid == pid) {
                return proc;
            }
        }

        return core::RichkwareError{core::ErrorCode::SystemError, "Process not found"};
    }

    core::Result<void> terminate_process_windows(std::uint32_t pid) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot open process"};
        }

        if (!TerminateProcess(hProcess, 0)) {
            CloseHandle(hProcess);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot terminate process"};
        }

        CloseHandle(hProcess);
        return core::Result<void>{};
    }

    core::Result<std::uint32_t> start_process_windows(const std::string& executable_path,
                                                    const std::vector<std::string>& arguments,
                                                    const std::string& working_directory) {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Build command line
        std::string cmd_line = executable_path;
        for (const auto& arg : arguments) {
            cmd_line += " " + arg;
        }

        // Convert strings to wide chars for Windows API
        std::vector<wchar_t> cmd_line_w(cmd_line.begin(), cmd_line.end());
        cmd_line_w.push_back(L'\0');

        std::vector<wchar_t> work_dir_w;
        if (!working_directory.empty()) {
            work_dir_w.assign(working_directory.begin(), working_directory.end());
            work_dir_w.push_back(L'\0');
        }

        if (!CreateProcess(NULL, cmd_line_w.data(), NULL, NULL, FALSE, 0, NULL,
                          work_dir_w.empty() ? NULL : work_dir_w.data(), &si, &pi)) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot create process"};
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return pi.dwProcessId;
    }

    core::Result<void> inject_dll_windows(std::uint32_t pid, const std::string& dll_path) {
        // Simplified DLL injection implementation
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!hProcess) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot open target process"};
        }

        // Allocate memory in target process
        size_t dll_path_size = (dll_path.size() + 1) * sizeof(char);
        LPVOID dll_path_addr = VirtualAllocEx(hProcess, NULL, dll_path_size, MEM_COMMIT, PAGE_READWRITE);
        if (!dll_path_addr) {
            CloseHandle(hProcess);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot allocate memory in target process"};
        }

        // Write DLL path to target process
        if (!WriteProcessMemory(hProcess, dll_path_addr, dll_path.c_str(), dll_path_size, NULL)) {
            VirtualFreeEx(hProcess, dll_path_addr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot write to target process memory"};
        }

        // Get LoadLibraryA address
        HMODULE kernel32 = GetModuleHandle("kernel32.dll");
        if (!kernel32) {
            VirtualFreeEx(hProcess, dll_path_addr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot get kernel32.dll handle"};
        }

        LPVOID load_library_addr = GetProcAddress(kernel32, "LoadLibraryA");
        if (!load_library_addr) {
            VirtualFreeEx(hProcess, dll_path_addr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot get LoadLibraryA address"};
        }

        // Create remote thread
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)load_library_addr,
                                          dll_path_addr, 0, NULL);
        if (!hThread) {
            VirtualFreeEx(hProcess, dll_path_addr, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot create remote thread"};
        }

        // Wait for thread to finish
        WaitForSingleObject(hThread, INFINITE);

        // Cleanup
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, dll_path_addr, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return core::Result<void>{};
    }

    bool is_system_process_windows(std::uint32_t pid) {
        return pid < 100 || pid == 4; // System, SMSS, etc.
    }

#elif defined(__linux__)
    core::Result<std::vector<ProcessInfo>> enumerate_processes_linux(const ProcessConfig& config) {
        std::vector<ProcessInfo> processes;

        DIR* proc_dir = opendir("/proc");
        if (!proc_dir) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot open /proc directory"};
        }

        struct dirent* entry;
        while ((entry = readdir(proc_dir)) != NULL) {
            // Check if directory name is numeric (PID)
            std::string dirname = entry->d_name;
            if (dirname.find_first_not_of("0123456789") != std::string::npos) {
                continue;
            }

            std::uint32_t pid = std::stoi(dirname);
            auto proc_info = get_process_info_linux(pid);

            if (proc_info) {
                const auto& info = proc_info.value();

                // Skip system processes if not requested
                if (!config.include_system_processes && info.is_system_process) {
                    continue;
                }

                processes.push_back(info);
            }
        }

        closedir(proc_dir);
        return processes;
    }

    core::Result<ProcessInfo> get_process_info_linux(std::uint32_t pid) {
        ProcessInfo info;
        info.pid = pid;

        std::string proc_path = "/proc/" + std::to_string(pid);

        // Read stat file
        std::ifstream stat_file(proc_path + "/stat");
        if (stat_file.is_open()) {
            std::string line;
            std::getline(stat_file, line);

            // Parse stat file (simplified)
            std::istringstream iss(line);
            std::string token;
            iss >> token; // pid
            iss >> token; // comm (name in parentheses)
            size_t start = token.find('(');
            size_t end = token.find(')');
            if (start != std::string::npos && end != std::string::npos) {
                info.name = token.substr(start + 1, end - start - 1);
            }
            iss >> token; // state
            iss >> token; // ppid
            info.parent_pid = std::stoi(token);
        }

        // Read exe link for executable path
        char exe_path[PATH_MAX];
        ssize_t len = readlink((proc_path + "/exe").c_str(), exe_path, sizeof(exe_path) - 1);
        if (len > 0) {
            exe_path[len] = '\0';
            info.executable_path = exe_path;
        }

        // Read status file for more info
        std::ifstream status_file(proc_path + "/status");
        if (status_file.is_open()) {
            std::string line;
            while (std::getline(status_file, line)) {
                if (line.substr(0, 4) == "Uid:") {
                    std::istringstream iss(line.substr(4));
                    uid_t uid;
                    iss >> uid;
                    struct passwd* pwd = getpwuid(uid);
                    if (pwd) {
                        info.user = pwd->pw_name;
                    }
                }
            }
        }

        // Check if system process
        info.is_system_process = (info.user == "root" && info.pid < 1000);

        return info;
    }

    core::Result<void> terminate_process_linux(std::uint32_t pid) {
        if (kill(pid, SIGTERM) == -1) {
            if (kill(pid, SIGKILL) == -1) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Cannot terminate process"};
            }
        }

        // Wait a bit for graceful shutdown
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return core::Result<void>{};
    }

    core::Result<std::uint32_t> start_process_linux(const std::string& executable_path,
                                                  const std::vector<std::string>& arguments,
                                                  const std::string& working_directory) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (!working_directory.empty()) {
                chdir(working_directory.c_str());
            }

            // Prepare arguments for execvp
            std::vector<char*> argv;
            argv.push_back(const_cast<char*>(executable_path.c_str()));
            for (const auto& arg : arguments) {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }
            argv.push_back(NULL);

            execvp(executable_path.c_str(), argv.data());

            // If exec fails
            exit(1);
        } else if (pid > 0) {
            // Parent process
            return pid;
        } else {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot fork process"};
        }
    }


#endif
};

ProcessManager::ProcessManager() : pimpl_(std::make_unique<Impl>()) {}
ProcessManager::~ProcessManager() = default;

core::Result<std::vector<ProcessInfo>> ProcessManager::enumerate_processes(const ProcessConfig& config) {
    return pimpl_->enumerate_processes(config);
}

core::Result<ProcessInfo> ProcessManager::get_process_info(std::uint32_t pid) {
    return pimpl_->get_process_info(pid);
}

core::Result<void> ProcessManager::terminate_process(std::uint32_t pid) {
    return pimpl_->terminate_process(pid);
}

core::Result<std::uint32_t> ProcessManager::start_process(const std::string& executable_path,
                                                        const std::vector<std::string>& arguments,
                                                        const std::string& working_directory) {
    return pimpl_->start_process(executable_path, arguments, working_directory);
}

bool ProcessManager::is_process_running(std::uint32_t pid) {
    return pimpl_->is_process_running(pid);
}

core::Result<std::vector<std::uint32_t>> ProcessManager::get_child_processes(std::uint32_t parent_pid) {
    return pimpl_->get_child_processes(parent_pid);
}

std::uint32_t ProcessManager::get_current_pid() const noexcept {
    return pimpl_->get_current_pid();
}

core::Result<void> ProcessManager::inject_dll(std::uint32_t pid, const std::string& dll_path) {
    return pimpl_->inject_dll(pid, dll_path);
}

} // namespace richkware::modules