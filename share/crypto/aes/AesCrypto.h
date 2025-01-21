#pragma once
#include <string>
#include <openssl/aes.h>

/**
 * 在AES-CBC模式中，原始数据需要被分割成块，每个块的大小为128位（16字节）
 * 如果原始数据不是128位的倍数，则需要进行填充（Padding）
 * 加密后的数据长度与填充后的数据长度相等
 * 密钥长度可选128位、192位或256位
 */

class AesCrypto
{
public:
	AesCrypto(const std::string& key);		// 传入密钥
	~AesCrypto() = default;
	
	std::string cbcEncrypt(const std::string& plaintext);	// 加密
	std::string cbcDecrypt(const std::string& ciphertext);	// 解密

private:
	AES_KEY m_encryptKey;		// 加密密钥调度
	AES_KEY m_decryptKey;		// 解密密钥调度
	std::string m_originalKey;	// 原始密钥
};