#include <cstring>
#include "Hash.h"
#include "../../SimpleLogger.h"

Hash::Hash(HashType type) : m_type(type)
{
	switch (m_type)
	{
	case HashType::MD5:
		MD5_Init(&m_md5);
		break;
	case HashType::SHA1:
		SHA1_Init(&m_sha1);
		break;
	case HashType::SHA224:
		SHA224_Init(&m_sha224);
		break;
	case HashType::SHA256:
		SHA256_Init(&m_sha256);
		break;
	case HashType::SHA384:
		SHA384_Init(&m_sha384);
		break;
	case HashType::SHA512:
		SHA512_Init(&m_sha512);
		break;
	}
}

void Hash::addData(const std::string& data)
{
	switch (m_type)
	{
	case HashType::MD5:
		MD5_Update(&m_md5, data.data(), data.length());
		break;
	case HashType::SHA1:
		SHA1_Update(&m_sha1, data.data(), data.length());
		break;
	case HashType::SHA224:
		SHA224_Update(&m_sha224, data.data(), data.length());
		break;
	case HashType::SHA256:
		SHA256_Update(&m_sha256, data.data(), data.length());
		break;
	case HashType::SHA384:
		SHA384_Update(&m_sha384, data.data(), data.length());
		break;
	case HashType::SHA512:
		SHA512_Update(&m_sha512, data.data(), data.length());
		break;
	}
}

std::string Hash::result()
{
	switch (m_type)
	{
	case HashType::MD5:
		return md5Result();
	case HashType::SHA1:
		return sha1Result();
	case HashType::SHA224:
		return sha224Result();
	case HashType::SHA256:
		return sha256Result();
	case HashType::SHA384:
		return sha384Result();
	case HashType::SHA512:
		return sha512Result();
	default:
		LOG_ERROR("Unsupported hash type");
		return std::string();
	}
}

std::string Hash::md5Result()
{
	unsigned char md5[MD5_DIGEST_LENGTH];
	MD5_Final(md5, &m_md5);

	char res[MD5_DIGEST_LENGTH * 2 + 1]{};
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		sprintf(&res[i*2], "%02x", md5[i]);
	}

	return std::string(res);
}

std::string Hash::sha1Result()
{
	unsigned char sha1[SHA_DIGEST_LENGTH];
	SHA1_Final(sha1, &m_sha1);

	char res[SHA_DIGEST_LENGTH * 2 + 1]{};
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		sprintf(&res[i * 2], "%02x", sha1[i]);
	}

	return std::string(res);
}

std::string Hash::sha224Result()
{
	unsigned char sha224[SHA224_DIGEST_LENGTH];
	SHA224_Final(sha224, &m_sha224);

	char res[SHA224_DIGEST_LENGTH * 2 + 1]{};
	for (int i = 0; i < SHA224_DIGEST_LENGTH; i++)
	{
		sprintf(&res[i * 2], "%02x", sha224[i]);
	}

	return std::string(res);
}

std::string Hash::sha256Result()
{
	unsigned char sha256[SHA256_DIGEST_LENGTH];
	SHA256_Final(sha256, &m_sha256);

	char res[SHA256_DIGEST_LENGTH * 2 + 1]{};
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(&res[i * 2], "%02x", sha256[i]);
	}
	return std::string(res);
}

std::string Hash::sha384Result()
{
	unsigned char sha384[SHA384_DIGEST_LENGTH];
	SHA384_Final(sha384, &m_sha384);

	char res[SHA384_DIGEST_LENGTH * 2 + 1]{};
	for (int i = 0; i < SHA384_DIGEST_LENGTH; i++)
	{
		sprintf(&res[i * 2], "%02x", sha384[i]);
	}

	return std::string(res);
}

std::string Hash::sha512Result()
{
	unsigned char sha512[SHA512_DIGEST_LENGTH];
	SHA512_Final(sha512, &m_sha512);

	char res[SHA512_DIGEST_LENGTH * 2 + 1]{};
	for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
	{
		sprintf(&res[i*2], "%02x", sha512[i]);
	}

	return std::string(res);
}