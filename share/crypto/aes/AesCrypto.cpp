#include <string>
#include <cstring>
#include <vector>
#include <iostream> 
#include <openssl/rand.h>
#include "AesCrypto.h"
#include "../../SimpleLogger.h"

// 传入密钥
AesCrypto::AesCrypto(const std::string& key)
{
	m_originalKey = key;
	if (m_originalKey.size() != 16 || m_originalKey.size() != 24 || m_originalKey.size() != 32)
	{
		LOG_ERROR("Invalid length of secret key");
	}

	AES_set_encrypt_key((const unsigned char*)m_originalKey.data(), m_originalKey.length() * 8, &m_encryptKey);
	AES_set_decrypt_key((const unsigned char*)m_originalKey.data(), m_originalKey.length() * 8, &m_decryptKey);
}

// 加密
std::string AesCrypto::cbcEncrypt(const std::string& plaintext)
{
	// 密文长度 = 明文长度 + 填充长度
	size_t plainLen = plaintext.length();								// 明文长度
	size_t paddingLen = AES_BLOCK_SIZE - (plainLen % AES_BLOCK_SIZE);	// 填充长度
	size_t cipherLen = plainLen + paddingLen;							// 密文长度
	
	// 生成随机iv(初始化向量)
    std::array<unsigned char, AES_BLOCK_SIZE> iv;
    if (RAND_bytes(iv.data(), AES_BLOCK_SIZE) != 1) 
	{
		LOG_ERROR("Failed to generate random IV");
		return std::string();
    }

	// 准备密文缓冲区，用于输出（包含了iv的存储空间）
	std::vector<unsigned char> ciphertext(AES_BLOCK_SIZE + cipherLen);

	// 复制iv到密文缓冲区
    std::copy(iv.begin(), iv.end(), ciphertext.begin());

	// 复制明文并进行PKCS#7填充
    std::copy(plaintext.begin(), plaintext.end(), ciphertext.begin() + AES_BLOCK_SIZE);
    std::fill(ciphertext.begin() + AES_BLOCK_SIZE + plainLen, ciphertext.end(), static_cast<unsigned char>(paddingLen));
	
	/**
	 * 执行AES的CBC加密
	 * 第一个参数为输入缓冲区，指向待加密的数据
	 * 第二个参数为输出缓冲区，用于存储加密后的数据
	 * 加密的话需要初始化向量iv的参与
	 */
    AES_cbc_encrypt(ciphertext.data() + AES_BLOCK_SIZE, ciphertext.data() + AES_BLOCK_SIZE, cipherLen,
                    &m_encryptKey, iv.data(), AES_ENCRYPT);

    return std::string(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
}

// 解密
std::string AesCrypto::cbcDecrypt(const std::string& ciphertext)
{
	if (ciphertext.length() < AES_BLOCK_SIZE) 
	{
        LOG_ERROR("Ciphertext is too short");
        return std::string();
    }

	size_t cipherLen = ciphertext.length() - AES_BLOCK_SIZE;	// 减去初始化向量的长度
    std::vector<unsigned char> plaintext(cipherLen);

	// 从密文中提取IV
    std::array<unsigned char, AES_BLOCK_SIZE> iv;
    std::copy(ciphertext.begin(), ciphertext.begin() + AES_BLOCK_SIZE, iv.begin());

	// 执行AES-CBC解密
    AES_cbc_encrypt(
        reinterpret_cast<const unsigned char*>(ciphertext.data()) + AES_BLOCK_SIZE,
        plaintext.data(),
        cipherLen,
        &m_decryptKey,
        iv.data(),
        AES_DECRYPT
    );

	// 检查并移除PKCS#7填充
	size_t paddingLen = plaintext[cipherLen - 1];

	// 检查填充长度是否在合理范围内
    if (paddingLen > 0 && paddingLen <= AES_BLOCK_SIZE) 
	{
        bool validPadding = true;
        for (size_t i = 0; i < paddingLen; ++i) 
		{
            if (plaintext[cipherLen - 1 - i] != paddingLen) 
			{
                validPadding = false;
                break;
            }
        }

        if (validPadding) 
		{
            // 只有在填充有效的情况下才移除填充
            size_t plaintextLen = cipherLen - paddingLen;
            return std::string(reinterpret_cast<const char*>(plaintext.data()), plaintextLen);
        } 
		else 
		{
            LOG_WARNING("Invalid padding detected, returning full decrypted data");
        }
    } 
	// 填充长度是不在合理范围内，说明没有填充
	else 
	{
        LOG_WARNING("No valid padding detected, returning full decrypted data");
    }
	
	// 如果填充无效或不存在，返回完整的解密数据
    return std::string(reinterpret_cast<const char*>(plaintext.data()), cipherLen);
}