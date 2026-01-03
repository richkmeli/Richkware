#include <gtest/gtest.h>
#include <richkware/core/types.hpp>
#include <richkware/crypto/cipher.hpp>

using namespace richkware;

class CryptoTest : public ::testing::Test {
protected:
    void SetUp() override {
        cipher_manager_ = std::make_unique<crypto::CipherManager>();
    }
    
    std::unique_ptr<crypto::CipherManager> cipher_manager_;
};

TEST_F(CryptoTest, EncryptDecryptString) {
    const std::string password = "test_password_123";
    const std::string plaintext = "Hello, World!";
    
    // Set password
    ASSERT_TRUE(cipher_manager_->set_password(password));
    
    // Encrypt
    auto encrypt_result = cipher_manager_->encrypt_string(plaintext);
    ASSERT_TRUE(encrypt_result);

    const std::string ciphertext = encrypt_result.value();
    EXPECT_NE(plaintext, ciphertext);
    EXPECT_FALSE(ciphertext.empty());

    // Decrypt
    auto decrypt_result = cipher_manager_->decrypt_string(ciphertext);
    ASSERT_TRUE(decrypt_result);
    
    const std::string decrypted = decrypt_result.value();
    EXPECT_EQ(plaintext, decrypted);
}

TEST_F(CryptoTest, GenerateRandomBytes) {
    const std::size_t size = 32;
    
    auto result1 = crypto::CipherManager::generate_random(size);
    ASSERT_TRUE(result1);
    EXPECT_EQ(size, result1.value().size());

    auto result2 = crypto::CipherManager::generate_random(size);
    ASSERT_TRUE(result2);
    EXPECT_EQ(size, result2.value().size());
    
    // Random bytes should be different
    EXPECT_NE(result1.value(), result2.value());
}

TEST_F(CryptoTest, InvalidPassword) {
    const std::string empty_password = "";
    
    auto result = cipher_manager_->set_password(empty_password);
    EXPECT_FALSE(result);
    EXPECT_EQ(core::ErrorCode::InvalidArgument, result.error().code());
}

class ConfigTest : public ::testing::Test {};

TEST_F(ConfigTest, DefaultValues) {
    core::Config config;
    
    EXPECT_EQ("Richkware", config.app_name);
    EXPECT_EQ("127.0.0.1", config.server_address);
    EXPECT_EQ(8080, config.server_port);
    EXPECT_TRUE(config.enable_encryption);
    EXPECT_TRUE(config.enable_stealth);
    EXPECT_EQ("info", config.log_level);
}

TEST_F(ConfigTest, CustomValues) {
    core::Config config{
        .app_name = "TestAgent",
        .encryption_key = "test_key",
        .server_address = "192.168.1.100",
        .server_port = 9999,
        .user_id = "test_user",
        .connection_timeout = std::chrono::milliseconds(30000),
        .enable_encryption = false,
        .enable_stealth = true,
        .log_level = "debug"
    };
    
    EXPECT_EQ("TestAgent", config.app_name);
    EXPECT_EQ("192.168.1.100", config.server_address);
    EXPECT_EQ(9999, config.server_port);
    EXPECT_FALSE(config.enable_encryption);
    EXPECT_EQ("debug", config.log_level);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}