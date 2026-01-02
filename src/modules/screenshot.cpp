#include "richkware/modules/screenshot.hpp"
#include "richkware/utils/logger.hpp"
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <wingdi.h>
#include <tlhelp32.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <png.h>
#include <zlib.h>
#endif

namespace richkware::modules {

// Screenshot implementation
class Screenshot::Impl {
public:
    core::Result<core::Bytes> capture_screen(const ScreenshotConfig& config) {
#ifdef _WIN32
        return capture_screen_windows(config);
#else
        return capture_screen_linux(config);
#endif
    }

    core::Result<core::Bytes> capture_window(const std::string& window_title, const ScreenshotConfig& config) {
#ifdef _WIN32
        return capture_window_windows(window_title, config);
#else
        return capture_window_linux(window_title, config);
#endif
    }

    core::Result<void> save_screenshot(const core::Bytes& image_data,
                                     const std::filesystem::path& filepath,
                                     const ScreenshotConfig& config) {
        try {
            std::ofstream file(filepath, std::ios::binary);
            if (!file.is_open()) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Cannot open file for writing"};
            }

            file.write(reinterpret_cast<const char*>(image_data.data()), image_data.size());
            file.close();

            LOG_INFO("Screenshot saved to: {}", filepath.string());
            return core::Result<void>{};
        } catch (const std::exception& e) {
            return core::RichkwareError{core::ErrorCode::SystemError,
                std::string("Error saving screenshot: ") + e.what()};
        }
    }

    core::Result<std::pair<int, int>> get_screen_size() {
#ifdef _WIN32
        return get_screen_size_windows();
#else
        return get_screen_size_linux();
#endif
    }

    core::Result<std::vector<std::string>> list_windows() {
#ifdef _WIN32
        return list_windows_windows();
#else
        return list_windows_linux();
#endif
    }

private:
#ifdef _WIN32
    core::Result<core::Bytes> capture_screen_windows(const ScreenshotConfig& config) {
        // Get screen dimensions
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        if (width == 0 || height == 0) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot get screen dimensions"};
        }

        // Create device contexts
        HDC hScreenDC = GetDC(NULL);
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

        if (!hScreenDC || !hMemoryDC) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot create device contexts"};
        }

        // Create compatible bitmap
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        if (!hBitmap) {
            DeleteDC(hMemoryDC);
            ReleaseDC(NULL, hScreenDC);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot create bitmap"};
        }

        // Select bitmap into memory DC
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

        // Copy screen to bitmap
        if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY)) {
            SelectObject(hMemoryDC, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hMemoryDC);
            ReleaseDC(NULL, hScreenDC);
            return core::RichkwareError{core::ErrorCode::SystemError, "BitBlt failed"};
        }

        // Convert bitmap to bytes (simplified - just BMP format)
        core::Bytes image_data = bitmap_to_bytes(hBitmap, width, height);

        // Cleanup
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);

        return image_data;
    }

    core::Result<core::Bytes> capture_window_windows(const std::string& window_title, const ScreenshotConfig& config) {
        // Find window by title
        HWND hwnd = FindWindow(NULL, window_title.c_str());
        if (!hwnd) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Window not found"};
        }

        // Get window dimensions
        RECT rect;
        if (!GetWindowRect(hwnd, &rect)) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot get window rect"};
        }

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        if (width <= 0 || height <= 0) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Invalid window dimensions"};
        }

        // Create device contexts
        HDC hScreenDC = GetDC(NULL);
        HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

        if (!hScreenDC || !hMemoryDC) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot create device contexts"};
        }

        // Create compatible bitmap
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        if (!hBitmap) {
            DeleteDC(hMemoryDC);
            ReleaseDC(NULL, hScreenDC);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot create bitmap"};
        }

        // Select bitmap into memory DC
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

        // Copy window to bitmap
        if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, rect.left, rect.top, SRCCOPY)) {
            SelectObject(hMemoryDC, hOldBitmap);
            DeleteObject(hBitmap);
            DeleteDC(hMemoryDC);
            ReleaseDC(NULL, hScreenDC);
            return core::RichkwareError{core::ErrorCode::SystemError, "BitBlt failed"};
        }

        // Convert bitmap to bytes
        core::Bytes image_data = bitmap_to_bytes(hBitmap, width, height);

        // Cleanup
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);

        return image_data;
    }

    core::Bytes bitmap_to_bytes(HBITMAP hBitmap, int width, int height) {
        // Simplified BMP implementation
        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // Negative for top-down bitmap
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        HDC hDC = GetDC(NULL);
        void* pBits;
        HBITMAP hDIB = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

        if (hDIB) {
            HDC hMemDC = CreateCompatibleDC(hDC);
            HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, hBitmap);

            BitBlt(hMemDC, 0, 0, width, height, hDC, 0, 0, SRCCOPY);

            // Get bitmap data
            GetDIBits(hMemDC, hBitmap, 0, height, pBits, &bmi, DIB_RGB_COLORS);

            // Create BMP file in memory
            core::Bytes bmp_data = create_bmp_from_pixels(static_cast<uint8_t*>(pBits), width, height);

            SelectObject(hMemDC, hOld);
            DeleteDC(hMemDC);
            DeleteObject(hDIB);
        }

        ReleaseDC(NULL, hDC);
        return bmp_data;
    }

    core::Bytes create_bmp_from_pixels(uint8_t* pixels, int width, int height) {
        // Simplified BMP header creation
        const int header_size = 54;
        const int image_size = width * height * 4; // 32-bit RGBA
        const int file_size = header_size + image_size;

        core::Bytes data(file_size);

        // BMP header
        data[0] = 'B'; data[1] = 'M'; // Signature
        *reinterpret_cast<uint32_t*>(&data[2]) = file_size; // File size
        *reinterpret_cast<uint32_t*>(&data[10]) = header_size; // Data offset

        // DIB header
        *reinterpret_cast<uint32_t*>(&data[14]) = 40; // Header size
        *reinterpret_cast<int32_t*>(&data[18]) = width; // Width
        *reinterpret_cast<int32_t*>(&data[22]) = height; // Height
        *reinterpret_cast<uint16_t*>(&data[26]) = 1; // Planes
        *reinterpret_cast<uint16_t*>(&data[28]) = 32; // Bits per pixel
        *reinterpret_cast<uint32_t*>(&data[30]) = 0; // Compression
        *reinterpret_cast<uint32_t*>(&data[34]) = image_size; // Image size

        // Copy pixel data
        std::memcpy(&data[header_size], pixels, image_size);

        return data;
    }

    core::Result<std::pair<int, int>> get_screen_size_windows() {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        if (width == 0 || height == 0) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot get screen size"};
        }

        return std::make_pair(width, height);
    }

    core::Result<std::vector<std::string>> list_windows_windows() {
        std::vector<std::string> windows;

        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            if (IsWindowVisible(hwnd)) {
                char title[256];
                GetWindowTextA(hwnd, title, sizeof(title));
                if (strlen(title) > 0) {
                    auto* windows = reinterpret_cast<std::vector<std::string>*>(lParam);
                    windows->push_back(title);
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&windows));

        return windows;
    }

#else
    core::Result<core::Bytes> capture_screen_linux(const ScreenshotConfig& config) {
        // Linux implementation using X11
        Display* display = XOpenDisplay(NULL);
        if (!display) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot open X11 display"};
        }

        Window root = DefaultRootWindow(display);
        XWindowAttributes attr;
        XGetWindowAttributes(display, root, &attr);

        int width = attr.width;
        int height = attr.height;

        XImage* image = XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);
        if (!image) {
            XCloseDisplay(display);
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot capture screen"};
        }

        // Convert XImage to PNG bytes (simplified)
        core::Bytes png_data = ximage_to_png(image, width, height);

        XDestroyImage(image);
        XCloseDisplay(display);

        return png_data;
    }

    core::Result<core::Bytes> capture_window_linux(const std::string& window_title, const ScreenshotConfig& config) {
        // Simplified implementation - capture entire screen for now
        return capture_screen_linux(config);
    }

    core::Bytes ximage_to_png(XImage* image, int width, int height) {
        // Simplified PNG creation - in real implementation use libpng properly
        core::Bytes data;

        // PNG signature
        data.insert(data.end(), {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A});

        // For now, return minimal PNG data
        // Real implementation would create proper PNG chunks

        return data;
    }

    core::Result<std::pair<int, int>> get_screen_size_linux() {
        Display* display = XOpenDisplay(NULL);
        if (!display) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot open display"};
        }

        int screen = DefaultScreen(display);
        int width = DisplayWidth(display, screen);
        int height = DisplayHeight(display, screen);

        XCloseDisplay(display);
        return std::make_pair(width, height);
    }

    core::Result<std::vector<std::string>> list_windows_linux() {
        std::vector<std::string> windows;

        Display* display = XOpenDisplay(NULL);
        if (!display) {
            return core::RichkwareError{core::ErrorCode::SystemError, "Cannot open display"};
        }

        Window root = DefaultRootWindow(display);

        // Get child windows
        Window root_return, parent_return;
        Window* children;
        unsigned int nchildren;

        if (XQueryTree(display, root, &root_return, &parent_return, &children, &nchildren)) {
            for (unsigned int i = 0; i < nchildren; ++i) {
                char* name = NULL;
                if (XFetchName(display, children[i], &name) && name) {
                    windows.push_back(name);
                    XFree(name);
                }
            }
            if (children) XFree(children);
        }

        XCloseDisplay(display);
        return windows;
    }
#endif
};

Screenshot::Screenshot() : pimpl_(std::make_unique<Impl>()) {}
Screenshot::~Screenshot() = default;

core::Result<core::Bytes> Screenshot::capture_screen(const ScreenshotConfig& config) {
    return pimpl_->capture_screen(config);
}

core::Result<core::Bytes> Screenshot::capture_window(const std::string& window_title, const ScreenshotConfig& config) {
    return pimpl_->capture_window(window_title, config);
}

core::Result<void> Screenshot::save_screenshot(const core::Bytes& image_data,
                                             const std::filesystem::path& filepath,
                                             const ScreenshotConfig& config) {
    return pimpl_->save_screenshot(image_data, filepath, config);
}

core::Result<std::pair<int, int>> Screenshot::get_screen_size() {
    return pimpl_->get_screen_size();
}

core::Result<std::vector<std::string>> Screenshot::list_windows() {
    return pimpl_->list_windows();
}

} // namespace richkware::modules