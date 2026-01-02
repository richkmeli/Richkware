#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <filesystem>

namespace richkware::modules {

struct ScreenshotConfig {
    std::string format{"png"};  // png, jpg, bmp
    int quality{90};            // JPEG quality (0-100)
    bool capture_cursor{true};  // Include mouse cursor
    std::string save_path;      // Auto-save path (optional)
};

/**
 * @brief Screenshot interface for screen capture
 */
class IScreenshot {
public:
    virtual ~IScreenshot() = default;

    /**
     * @brief Capture entire screen
     * @param config Screenshot configuration
     * @return Captured image data as bytes or error
     */
    [[nodiscard]] virtual core::Result<core::Bytes> capture_screen(
        const ScreenshotConfig& config = {}) = 0;

    /**
     * @brief Capture specific window
     * @param window_title Title of window to capture
     * @param config Screenshot configuration
     * @return Captured image data as bytes or error
     */
    [[nodiscard]] virtual core::Result<core::Bytes> capture_window(
        const std::string& window_title,
        const ScreenshotConfig& config = {}) = 0;

    /**
     * @brief Save screenshot to file
     * @param image_data Image data
     * @param filepath Output file path
     * @param config Screenshot configuration
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> save_screenshot(
        const core::Bytes& image_data,
        const std::filesystem::path& filepath,
        const ScreenshotConfig& config = {}) = 0;

    /**
     * @brief Get screen dimensions
     * @return Screen width and height or error
     */
    [[nodiscard]] virtual core::Result<std::pair<int, int>> get_screen_size() = 0;

    /**
     * @brief List available windows
     * @return List of window titles or error
     */
    [[nodiscard]] virtual core::Result<std::vector<std::string>> list_windows() = 0;
};

/**
 * @brief Screenshot implementation
 */
class Screenshot : public IScreenshot {
public:
    Screenshot();
    ~Screenshot() override;

    // Non-copyable, movable
    Screenshot(const Screenshot&) = delete;
    Screenshot& operator=(const Screenshot&) = delete;
    Screenshot(Screenshot&&) = default;
    Screenshot& operator=(Screenshot&&) = default;

    [[nodiscard]] core::Result<core::Bytes> capture_screen(
        const ScreenshotConfig& config = {}) override;

    [[nodiscard]] core::Result<core::Bytes> capture_window(
        const std::string& window_title,
        const ScreenshotConfig& config = {}) override;

    [[nodiscard]] core::Result<void> save_screenshot(
        const core::Bytes& image_data,
        const std::filesystem::path& filepath,
        const ScreenshotConfig& config = {}) override;

    [[nodiscard]] core::Result<std::pair<int, int>> get_screen_size() override;

    [[nodiscard]] core::Result<std::vector<std::string>> list_windows() override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::modules