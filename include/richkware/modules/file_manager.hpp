#pragma once

#include "../core/types.hpp"
#include <string>
#include <filesystem>
#include <vector>
#include <memory>

namespace richkware::modules {

struct FileInfo {
    std::string name;
    std::string path;
    std::uint64_t size{0};
    core::TimePoint modified_time;
    bool is_directory{false};
};

struct UploadRequest {
    std::string local_path;
    std::string remote_url;
    std::string encryption_key;
};

struct DownloadRequest {
    std::string remote_url;
    std::string local_path;
    std::string decryption_key;
};

/**
 * @brief Interface for file operations
 */
class IFileManager {
public:
    virtual ~IFileManager() = default;

    /**
     * @brief List files in directory
     * @param path Directory path
     * @return List of files or error
     */
    [[nodiscard]] virtual core::Result<std::vector<FileInfo>> list_directory(
        const std::filesystem::path& path) = 0;

    /**
     * @brief Read file content
     * @param path File path
     * @return File content as bytes or error
     */
    [[nodiscard]] virtual core::Result<core::Bytes> read_file(
        const std::filesystem::path& path) = 0;

    /**
     * @brief Write file content
     * @param path File path
     * @param content Content to write
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> write_file(
        const std::filesystem::path& path,
        const core::Bytes& content) = 0;

    /**
     * @brief Delete file or directory
     * @param path Path to delete
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> delete_path(
        const std::filesystem::path& path) = 0;

    /**
     * @brief Upload file to remote server
     * @param request Upload request
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> upload_file(const UploadRequest& request) = 0;

    /**
     * @brief Download file from remote server
     * @param request Download request
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> download_file(const DownloadRequest& request) = 0;

    /**
     * @brief Search for files matching pattern
     * @param directory Directory to search in
     * @param pattern Search pattern (supports wildcards)
     * @param recursive Search recursively
     * @return List of matching files or error
     */
    [[nodiscard]] virtual core::Result<std::vector<FileInfo>> search_files(
        const std::filesystem::path& directory,
        const std::string& pattern,
        bool recursive = false) = 0;

    /**
     * @brief Get file information
     * @param path File path
     * @return File information or error
     */
    [[nodiscard]] virtual core::Result<FileInfo> get_file_info(
        const std::filesystem::path& path) = 0;
};

/**
 * @brief File manager implementation
 */
class FileManager : public IFileManager {
public:
    FileManager();
    ~FileManager() override;

    // Non-copyable, movable
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;
    FileManager(FileManager&&) = default;
    FileManager& operator=(FileManager&&) = default;

    [[nodiscard]] core::Result<std::vector<FileInfo>> list_directory(
        const std::filesystem::path& path) override;

    [[nodiscard]] core::Result<core::Bytes> read_file(
        const std::filesystem::path& path) override;

    [[nodiscard]] core::Result<void> write_file(
        const std::filesystem::path& path,
        const core::Bytes& content) override;

    [[nodiscard]] core::Result<void> delete_path(
        const std::filesystem::path& path) override;

    [[nodiscard]] core::Result<void> upload_file(const UploadRequest& request) override;

    [[nodiscard]] core::Result<void> download_file(const DownloadRequest& request) override;

    [[nodiscard]] core::Result<std::vector<FileInfo>> search_files(
        const std::filesystem::path& directory,
        const std::string& pattern,
        bool recursive = false) override;

    [[nodiscard]] core::Result<FileInfo> get_file_info(
        const std::filesystem::path& path) override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::modules