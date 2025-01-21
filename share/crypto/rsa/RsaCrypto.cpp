#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "RsaCrypto.h"
#include "../../SimpleLogger.h"


RsaCrypto::RsaCrypto() 
	: m_publicKey(RSA_new(), &RSA_free), m_privateKey(RSA_new(), &RSA_free) 
{
    // 也可以将&RSA_free写成RSA_free，RSA_free函数名将隐式转化为函数指针
}

// 从pem文件中读取公钥或者私钥
RsaCrypto::RsaCrypto(std::string_view pemFile, bool isPublic)
    : m_publicKey(RSA_new(), &RSA_free), m_privateKey(RSA_new(), &RSA_free) {
    try {
        std::unique_ptr<BIO, decltype(&BIO_free)> bio(BIO_new_file(pemFile.data(), "r"), BIO_free);
        if (!bio) {
            throw std::runtime_error("Failed to open PEM file: " + std::string(pemFile));
        }

        if (isPublic) {
            /**
             * 不能写成PEM_read_bio_RSAPublicKey(bio.get(), &m_publicKey.get(), nullptr, nullptr)
             * 因为m_publicKey.get()得到的是匿名变量，匿名的是右值，不能取地址
             */
            RSA* rsa_ptr = m_publicKey.get();
            if (!PEM_read_bio_RSAPublicKey(bio.get(), &rsa_ptr, nullptr, nullptr)) {
                throw std::runtime_error("Failed to read public key from: " + std::string(pemFile));
            }
        } else {
            RSA* rsa_ptr = m_privateKey.get();
            if (!PEM_read_bio_RSAPrivateKey(bio.get(), &rsa_ptr, nullptr, nullptr)) {
                throw std::runtime_error("Failed to read private key from: " + std::string(pemFile));
            }
        }
    } catch (const std::exception& e) {
        // 记录日志
        LOG_ERROR("Error in RsaCrypto constructor: " + std::string(e.what()));
        // 重新抛出异常
        throw;
    }
}

// 生成密钥对
bool RsaCrypto::generateRsaKey(int bits, std::string_view pubKeyFile, std::string_view priKeyFile){
	if (bits % 1024 != 0 || bits < 2048) {
		LOG_ERROR("Invalid key length. Must be a multiple of 1024 and at least 2048 bits.");
        return false;
    }

	// 使用unique_ptr管理RSA和BIGNUM实例
    std::unique_ptr<RSA, decltype(&RSA_free)> rsa(RSA_new(), RSA_free);
    std::unique_ptr<BIGNUM, decltype(&BN_free)> e(BN_new(), BN_free);
	if (!rsa || !e) {
		LOG_ERROR("Failed to allocate RSA or BIGNUM");
        return false;
    }

    /**
     * 设置公钥指数，RSA_F4的值为65537
     * 公钥由两部分组成：模数和公钥指数
     */
	// 设置公钥指数，RSA_F4的值为65537
    if (BN_set_word(e.get(), RSA_F4) != 1)	{
		LOG_ERROR("Failed to set public exponent");
        return false;
    }

	// 生成密钥对
    if (RSA_generate_key_ex(rsa.get(), bits, e.get(), nullptr) != 1) {
		LOG_ERROR("Failed to generate RSA key pair");
        return false;
    }

	// 写入公钥文件
    if (!writeRsaKeyToFile(rsa.get(), pubKeyFile, true)) {
        return false;
    }

    // 写入私钥文件
    if (!writeRsaKeyToFile(rsa.get(), priKeyFile, false)) {
        return false;
    }

	// 更新成员变量
    m_publicKey.reset(RSAPublicKey_dup(rsa.get()));
    m_privateKey.reset(RSAPrivateKey_dup(rsa.get()));

    if (!m_publicKey || !m_privateKey) {
		LOG_ERROR("Failed to duplicate RSA keys");
        return false;
    }

    return true;
}


// 辅助函数：写入RSA密钥到文件
bool RsaCrypto::writeRsaKeyToFile(RSA* rsa, std::string_view filename, bool isPublic){
    std::unique_ptr<BIO, decltype(&BIO_free)> bio(BIO_new_file(filename.data(), "w"), BIO_free);
    if (!bio) {
		LOG_ERROR("Failed to create file: " + std::string(filename));
        return false;
    }

    int result;
    if (isPublic) {
        result = PEM_write_bio_RSAPublicKey(bio.get(), rsa);
    } else {
        result = PEM_write_bio_RSAPrivateKey(bio.get(), rsa, nullptr, nullptr, 0, nullptr, nullptr);
    }

    if (result != 1) {
		std::ostringstream errorMsg;
    	errorMsg << "Failed to write " << (isPublic ? "public" : "private") 
            	 << " key to file: " << filename;
    	LOG_ERROR(errorMsg.str());

        return false;
    }

    return true;
}

// 用公钥加密数据
std::string RsaCrypto::publicKeyEncrypt(std::string data)
{
	if (!m_publicKey) {
        LOG_ERROR("Public key is not initialized");
        return "";
    }

	int keyLen = RSA_size(m_publicKey.get());
    if (data.length() > static_cast<size_t>(keyLen) - 11) {
        LOG_ERROR("Data too long for RSA public key");
        return "";
    }
	std::vector<unsigned char> cipher(keyLen);

	int ret = RSA_public_encrypt(data.length(), 
                                 reinterpret_cast<const unsigned char*>(data.data()),
                                 cipher.data(), 
                                 m_publicKey.get(), 
                                 RSA_PKCS1_PADDING);
    if (ret == -1) {
        LOG_ERROR("Public key encryption failed");
        ERR_print_errors_fp(stderr);
        return "";
    }

    return toBase64(reinterpret_cast<const char*>(cipher.data()), ret);
}

// 要加密的数据的最大长度 = 密钥长度 - 11
std::string RsaCrypto::privateKeyDecrypt(std::string encData)
{
	if (!m_privateKey) {
        LOG_ERROR("Private key is not initialized");
        return "";
    }

    std::vector<unsigned char> decodedData = fromBase64(encData);
    if (decodedData.empty()) {
        LOG_ERROR("Failed to decode Base64 data");
        return "";
    }

    int keyLen = RSA_size(m_privateKey.get());
    if (decodedData.size() != static_cast<size_t>(keyLen)) {
        LOG_ERROR("Invalid encrypted data size");
        return "";
    }

    std::vector<unsigned char> decrypted(keyLen);

    int ret = RSA_private_decrypt(keyLen, 
                                  decodedData.data(),
                                  decrypted.data(), 
                                  m_privateKey.get(),
                                  RSA_PKCS1_PADDING);
    if (ret == -1) {
        LOG_ERROR("Private key decryption failed");
        ERR_print_errors_fp(stderr);
        return "";
    }

    return std::string(reinterpret_cast<char*>(decrypted.data()), ret);
}

// data为要签名的数据，数据的最大长度 = 密钥长度 - 11
std::string RsaCrypto::createSignature(std::string_view data, SignLevel level){
    if (!m_privateKey) {
        LOG_ERROR("Private key is not initialized");
        return "";
    }

    // 计算签名所需的缓冲区大小
    size_t signLen = RSA_size(m_privateKey.get());
    std::vector<unsigned char> signBuf(signLen);

    // 创建签名
    unsigned int actualSignLen;
    int result = RSA_sign(static_cast<int>(level), 
                          reinterpret_cast<const unsigned char*>(data.data()), 
                          data.length(), 
                          signBuf.data(), 
                          &actualSignLen, 
                          m_privateKey.get());

    if (result != 1) {
        LOG_ERROR("Failed to create signature");
        return "";
    }

    // 将签名转换为 Base64 编码
    return toBase64(reinterpret_cast<const char*>(signBuf.data()), actualSignLen);
}

bool RsaCrypto::verifySignature(std::string_view data, std::string_view signatureBase64, SignLevel level){
    if (!m_publicKey) {
        LOG_ERROR("Public key is not initialized");
        return false;
    }

    // 解码 Base64 签名
    std::vector<unsigned char> signature = fromBase64(signatureBase64);
    if (signature.empty()) {
        LOG_ERROR("Failed to decode Base64 signature");
        return false;
    }

    // 验证签名
    int result = RSA_verify(static_cast<int>(level),
                            reinterpret_cast<const unsigned char*>(data.data()),
                            data.length(),
                            signature.data(),
                            signature.size(),
                            m_publicKey.get());

    if (result != 1) {
        LOG_ERROR("Signature verification failed");
        ERR_print_errors_fp(stderr);
        return false;
    }

    return true;
}


std::string RsaCrypto::toBase64(const char* data, int length){
    if (data == nullptr || length <= 0) {
        LOG_ERROR("Invalid input data for Base64 encoding");
        return std::string();
    }

	// 创建BIO实例
    std::unique_ptr<BIO, decltype(&BIO_free_all)> b64(BIO_new(BIO_f_base64()), BIO_free_all);   // bs64对象，用于base64编码
    // 创建内存 BIO
    BIO* mem = BIO_new(BIO_s_mem());
    if (!mem) {
        LOG_ERROR("Failed to create memory BIO");
        return std::string();
    }

    /**
     * 将两个BIO实例添加到链表中，bs64->mem
     * 写入的bs64数据会自动进行base64编码，然后写入mem中
     * 读取的mem数据则会自动进行base64解码，然后返回原始数据
     */
	BIO* bio = BIO_push(b64.get(), mem);
	
	// 禁用换行
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

	// 数据最后会自动写入bs64链的末尾，就是mem中
    int written = BIO_write(bio, data, length);
    if (written <= 0) {
        LOG_ERROR("Failed to write data for Base64 encoding");
        return std::string();
    }
    BIO_flush(bio);

	// 得到内存对象指针
	BUF_MEM* bufferPtr;
	BIO_get_mem_ptr(bio, &bufferPtr);	// 虽然数据已经写入到mem中，但是因为是链表，需要从头开始访问

	return std::string(bufferPtr->data, bufferPtr->length);
}

std::vector<unsigned char> RsaCrypto::fromBase64(std::string_view str){
    if (str.empty()) {
        LOG_ERROR("Empty input string for Base64 decoding");
        return {};
    }

    // 创建base64 BIO
    std::unique_ptr<BIO, decltype(&BIO_free_all)> base64(BIO_new(BIO_f_base64()), BIO_free_all);
    if (!base64) {
        LOG_ERROR("Failed to create Base64 BIO");
        return {};
    }

    // 创建内存 BIO
    BIO* mem = BIO_new_mem_buf(str.data(), str.length());
    if (!mem) {
        LOG_ERROR("Failed to create memory BIO");
        return {};
    }

    // 将内存BIO推入base64 BIO
    BIO_push(base64.get(), mem);

    // 禁用换行
    BIO_set_flags(base64.get(), BIO_FLAGS_BASE64_NO_NL);

    // 创建缓冲区。Base64 解码后的数据通常比编码前小，所以使用 3/4 的大小通常足够
    std::vector<unsigned char> buffer(str.length() * 3 / 4 + 1);

    // 读取解码后的数据
    int decoded_size = BIO_read(base64.get(), buffer.data(), static_cast<int>(buffer.size()));
    if (decoded_size <= 0) {
        LOG_ERROR("Failed to decode Base64 data");
        return {};
    }

    // 调整buffer大小以匹配实际解码的数据量
    buffer.resize(decoded_size);

    return buffer;
}