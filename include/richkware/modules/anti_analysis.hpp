#pragma once

#include "../core/types.hpp"
#include <memory>
#include <vector>
#include <string>

namespace richkware::modules {

struct AntiAnalysisResult {
    bool debugger_detected{false};
    bool virtual_machine_detected{false};
    bool sandbox_detected{false};
    std::vector<std::string> detection_methods;
};

/**
 * @brief Anti-analysis interface for detecting debuggers and VMs
 */
class IAntiAnalysis {
public:
    virtual ~IAntiAnalysis() = default;

    /**
     * @brief Perform comprehensive analysis detection
     * @return Detection results
     */
    [[nodiscard]] virtual AntiAnalysisResult detect_analysis() = 0;

    /**
     * @brief Check if running under debugger
     * @return true if debugger detected
     */
    [[nodiscard]] virtual bool is_debugger_present() = 0;

    /**
     * @brief Check if running in virtual machine
     * @return true if VM detected
     */
    [[nodiscard]] virtual bool is_virtual_machine() = 0;

    /**
     * @brief Check if running in sandbox
     * @return true if sandbox detected
     */
    [[nodiscard]] virtual bool is_sandbox() = 0;

    /**
     * @brief Attempt to evade analysis (delay, obfuscation, etc.)
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> evade_analysis() = 0;
};

/**
 * @brief Anti-analysis implementation
 */
class AntiAnalysis : public IAntiAnalysis {
public:
    AntiAnalysis();
    ~AntiAnalysis() override;

    // Non-copyable, movable
    AntiAnalysis(const AntiAnalysis&) = delete;
    AntiAnalysis& operator=(const AntiAnalysis&) = delete;
    AntiAnalysis(AntiAnalysis&&) = default;
    AntiAnalysis& operator=(AntiAnalysis&&) = default;

    [[nodiscard]] AntiAnalysisResult detect_analysis() override;
    [[nodiscard]] bool is_debugger_present() override;
    [[nodiscard]] bool is_virtual_machine() override;
    [[nodiscard]] bool is_sandbox() override;
    [[nodiscard]] core::Result<void> evade_analysis() override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::modules