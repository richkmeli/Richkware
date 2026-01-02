#include "richkware/modules/file_manager.hpp"
#include "richkware/crypto/cipher.hpp"
#include "richkware/network/client.hpp"
#include "richkware/utils/logger.hpp"
#include <fstream>
#include <filesystem>
#include <regex>
#include <algorithm>

namespace fs = std::filesystem;

namespace richkware::modules {

// FileManager implementation
class FileManager::Impl {
public:
    crypto::CipherManager cipher_;
    std::shared_ptr<network::HttpsClient> http_client_;

    Impl() {
        http_client_ = std::make_shared<network::HttpsClient>();
        // Set cipher password for file operations
        (void)cipher_.set_password("file_operation_key");
    }

    core::Result<std::vector<FileInfo>> list_directory(const fs::path& path) {
        try {
            std::vector<FileInfo> files;
            if (!fs::exists(path) || !fs::is_directory(path)) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Path does not exist or is not a directory"};
            }

            for (const auto& entry : fs::directory_iterator(path)) {
                FileInfo info;
                info.name = entry.path().filename().string();
                info.path = entry.path().string();
                info.size = entry.is_regular_file() ? entry.file_size() : 0;
                info.is_directory = entry.is_directory();

                if (entry.exists()) {
                    info.modified_time = std::chrono::steady_clock::now(); // Simplified for now
                }

                files.push_back(info);
            }

            // Sort by name
            std::sort(files.begin(), files.end(),
                     [](const FileInfo& a, const FileInfo& b) { return a.name < b.name; });

            return files;
        } catch (const fs::filesystem_error& e) {
            return core::RichkwareError{core::ErrorCode::SystemError, std::string("Filesystem error: ") + e.what()};
        }
    }

    core::Result<core::Bytes> read_file(const fs::path& path) {
        try {
            if (!fs::exists(path) || !fs::is_regular_file(path)) {
                return core::RichkwareError{core::ErrorCode::SystemError, "File does not exist or is not a regular file"};
            }

            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Failed to open file for reading"};
            }

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            core::Bytes buffer(size);
            if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Failed to read file content"};
            }

            return buffer;
        } catch (const std::exception& e) {
            return core::RichkwareError{core::ErrorCode::SystemError, std::string("Error reading file: ") + e.what()};
        }
    }

    core::Result<void> write_file(const fs::path& path, const core::Bytes& content) {
        try {
            // Create parent directories if they don't exist
            fs::create_directories(path.parent_path());

            std::ofstream file(path, std::ios::binary | std::ios::trunc);
            if (!file.is_open()) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Failed to open file for writing"};
            }

            file.write(reinterpret_cast<const char*>(content.data()), content.size());
            if (!file) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Failed to write file content"};
            }

            return core::Result<void>{};
        } catch (const std::exception& e) {
            return core::RichkwareError{core::ErrorCode::SystemError, std::string("Error writing file: ") + e.what()};
        }
    }

    core::Result<void> delete_path(const fs::path& path) {
        try {
            if (!fs::exists(path)) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Path does not exist"};
            }

            if (fs::is_directory(path)) {
                fs::remove_all(path);
            } else {
                fs::remove(path);
            }

            return core::Result<void>{};
        } catch (const fs::filesystem_error& e) {
            return core::RichkwareError{core::ErrorCode::SystemError, std::string("Error deleting path: ") + e.what()};
        }
    }

    core::Result<void> upload_file(const UploadRequest& request) {
        // Read file
        auto content_result = read_file(request.local_path);
        if (!content_result) {
            return content_result.error();
        }

        // Encrypt content if key provided
        core::Bytes data_to_send = content_result.value();
        if (!request.encryption_key.empty()) {
            (void)cipher_.set_password(request.encryption_key);
            auto encrypted = cipher_.encrypt_string(std::string(data_to_send.begin(), data_to_send.end()));
            if (!encrypted) {
                return encrypted.error();
            }
            data_to_send = core::Bytes(encrypted.value().begin(), encrypted.value().end());
        }

        // TODO: Implement actual HTTP upload
        LOG_INFO("File upload not yet implemented: {} -> {}", request.local_path, request.remote_url);
        return core::Result<void>{};
    }

    core::Result<void> download_file(const DownloadRequest& request) {
        // TODO: Implement actual HTTP download
        LOG_INFO("File download not yet implemented: {} -> {}", request.remote_url, request.local_path);
        return core::Result<void>{};
    }

    core::Result<std::vector<FileInfo>> search_files(const fs::path& directory, const std::string& pattern, bool recursive) {
        try {
            std::vector<FileInfo> results;

            if (recursive) {
                for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                    if (!entry.is_regular_file()) continue;

                    std::string filename = entry.path().filename().string();

                    // Simple wildcard matching (* and ?)
                    if (matches_pattern(filename, pattern)) {
                        FileInfo info;
                        info.name = filename;
                        info.path = entry.path().string();
                        info.size = entry.file_size();
                        info.is_directory = false;
                        info.modified_time = std::chrono::steady_clock::now();
                        results.push_back(info);
                    }
                }
            } else {
                for (const auto& entry : fs::directory_iterator(directory)) {
                    if (!entry.is_regular_file()) continue;

                    std::string filename = entry.path().filename().string();

                    // Simple wildcard matching (* and ?)
                    if (matches_pattern(filename, pattern)) {
                        FileInfo info;
                        info.name = filename;
                        info.path = entry.path().string();
                        info.size = entry.file_size();
                        info.is_directory = false;
                        info.modified_time = std::chrono::steady_clock::now();
                        results.push_back(info);
                    }
                }
            }

            return results;
        } catch (const std::exception& e) {
            return core::RichkwareError{core::ErrorCode::SystemError, std::string("Error searching files: ") + e.what()};
        }
    }

    core::Result<FileInfo> get_file_info(const fs::path& path) {
        try {
            if (!fs::exists(path)) {
                return core::RichkwareError{core::ErrorCode::SystemError, "Path does not exist"};
            }

            FileInfo info;
            info.name = path.filename().string();
            info.path = path.string();
            info.is_directory = fs::is_directory(path);

            if (fs::is_regular_file(path)) {
                info.size = fs::file_size(path);
            }

            info.modified_time = std::chrono::steady_clock::now(); // Simplified

            return info;
        } catch (const fs::filesystem_error& e) {
            return core::RichkwareError{core::ErrorCode::SystemError, std::string("Error getting file info: ") + e.what()};
        }
    }

private:
    bool matches_pattern(const std::string& text, const std::string& pattern) {
        // Convert wildcard pattern to regex
        std::string regex_pattern = pattern;
        // Escape special regex characters except * and ?
        regex_pattern = std::regex_replace(regex_pattern, std::regex(R"([\.\+\^\$\(\)\[\]\{\}\|\\])"), R"(\$&)");
        // Convert wildcards to regex
        regex_pattern = std::regex_replace(regex_pattern, std::regex(R"(\*)"), ".*");
        regex_pattern = std::regex_replace(regex_pattern, std::regex(R"(\?)"), ".");

        std::regex regex(regex_pattern, std::regex_constants::icase);
        return std::regex_match(text, regex);
    }
};

FileManager::FileManager() : pimpl_(std::make_unique<Impl>()) {}
FileManager::~FileManager() = default;

core::Result<std::vector<FileInfo>> FileManager::list_directory(const fs::path& path) {
    return pimpl_->list_directory(path);
}

core::Result<core::Bytes> FileManager::read_file(const fs::path& path) {
    return pimpl_->read_file(path);
}

core::Result<void> FileManager::write_file(const fs::path& path, const core::Bytes& content) {
    return pimpl_->write_file(path, content);
}

core::Result<void> FileManager::delete_path(const fs::path& path) {
    return pimpl_->delete_path(path);
}

core::Result<void> FileManager::upload_file(const UploadRequest& request) {
    return pimpl_->upload_file(request);
}

core::Result<void> FileManager::download_file(const DownloadRequest& request) {
    return pimpl_->download_file(request);
}

core::Result<std::vector<FileInfo>> FileManager::search_files(const fs::path& directory, const std::string& pattern, bool recursive) {
    return pimpl_->search_files(directory, pattern, recursive);
}

core::Result<FileInfo> FileManager::get_file_info(const fs::path& path) {
    return pimpl_->get_file_info(path);
}

} // namespace richkware::modules