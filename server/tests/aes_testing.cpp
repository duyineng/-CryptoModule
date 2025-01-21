#include <string>
#include <gtest/gtest.h>
#include "../../share/crypto/aes/AesCrypto.h"

TEST(CRYPTO, AES)
{
    // 测试正常情况
    {
        std::string key = "0123456789abcdef"; // 16字节密钥
        AesCrypto aes(key);
        
        std::string plaintext = "Hello, World!";
        std::string encrypted = aes.cbcEncrypt(plaintext);
        std::string decrypted = aes.cbcDecrypt(encrypted);
        
        EXPECT_EQ(plaintext, decrypted);
    }
}