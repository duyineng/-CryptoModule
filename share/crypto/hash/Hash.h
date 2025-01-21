#pragma once
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <string>
#include <vector>
#include <array>

enum class HashType 
{ 
	MD5,		// MD5加密后的字符串长度16字节
	SHA1, 		// SHA1加密后的字符串长度20字节
	SHA224, 	// SHA224加密后的字符串长度28字节
	SHA256, 	// SHA256加密后的字符串长度32字节
	SHA384, 	// SHA384加密后的字符串长度48字节
	SHA512 		// SHA512加密后的字符串长度64字节
};

class Hash
{
public:
	explicit Hash(HashType type);
	~Hash() = default;
	void addData(const std::string& data);
	std::string result();

private:
	std::string md5Result();
	std::string sha1Result();
	std::string sha224Result();
	std::string sha256Result();
	std::string sha384Result();
	std::string sha512Result();

	HashType m_type;
	MD5_CTX m_md5;
	SHA_CTX m_sha1;
	SHA256_CTX m_sha224;
	SHA256_CTX m_sha256;
	SHA512_CTX m_sha384;
	SHA512_CTX m_sha512;
};