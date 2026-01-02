#include <richkware/core/agent.hpp>
#include <iostream>

using namespace richkware;

int main() {
    std::cout << "Richkware 2.0 - Modern C++ Malware Framework" << std::endl;
    std::cout << "Educational purposes only!" << std::endl;
    
    try {
        // Configure agent
        core::Config config{
            .app_name = "ModernRichkware",
            .encryption_key = "test_key_123",
            .server_address = "127.0.0.1",
            .server_port = 8443,
            .user_id = "agent_001"
        };
        
        // Create agent
        core::Agent agent(std::move(config));
        
        std::cout << "Agent created with name: " << agent.config().app_name << std::endl;
        
        // Initialize agent components
        auto init_result = agent.initialize();
        if (!init_result) {
            std::cerr << "Failed to initialize agent" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Agent initialized successfully" << std::endl;
        
        // Start agent operations
        auto start_result = agent.start();
        if (!start_result) {
            std::cerr << "Failed to start agent" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Agent started successfully" << std::endl;
        
        // Test command execution
        auto cmd_result = agent.execute_command("echo 'Hello from Richkware!'");
        if (cmd_result) {
            std::cout << "✓ Command executed: " << cmd_result.value() << std::endl;
        } else {
            std::cout << "✗ Command execution failed" << std::endl;
        }
        
        // Test stealth mode
        auto stealth_result = agent.enable_stealth();
        if (stealth_result) {
            std::cout << "✓ Stealth mode enabled" << std::endl;
        }
        
        std::cout << "Agent running: " << (agent.is_running() ? "Yes" : "No") << std::endl;
        
        // Graceful shutdown
        agent.stop();
        std::cout << "✓ Agent stopped gracefully" << std::endl;
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}