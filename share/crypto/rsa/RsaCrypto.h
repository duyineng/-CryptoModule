#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <openssl/evp.h>

#ifdef _WIN32
extern "C"
{
#include <openssl/applink.c>
};
#endif

enum class SignLevel
{
	Level1 = NID_md5,	// NID，Numeric Identifier，数字标识符
	Level2 = NID_sha1,
	Level3 = NID_sha224,
	Level4 = NID_sha256,
	Level5 = NID_sha384,
	Level6 = NID_sha512
};

class RsaCrypto
{
public:
	RsaCrypto();
	RsaCrypto(std::string_view pemFile, bool isPublic);		// 从pem文件中读取公钥或者私钥
	~RsaCrypto() = default;

	/**
	 * 生成RSA密钥对，指定的密钥长度为1024位的整数倍，加密后的密文长度就等于密钥长度
	 * 公钥和私钥的模数长度相同，但是私钥包含更多信息，所以私钥文件通常比公钥文件大
	 */
	bool generateRsaKey(int bits, std::string_view pubKeyFile = "public.pem", std::string_view priKeyFile = "private.pem");
	std::string publicKeyEncrypt(std::string data);		// 公钥加密
	std::string privateKeyDecrypt(std::string encData);	// 私钥解密
	std::string createSignature(std::string_view data , SignLevel level = SignLevel::Level3);	// 签名
	bool verifySignature(std::string_view data, std::string_view signData, SignLevel level = SignLevel::Level3);	// 校验

private:
	bool writeRsaKeyToFile(RSA* rsa, std::string_view filename, bool isPublic);
	std::string toBase64(const char* data, int length);
	std::vector<unsigned char> fromBase64(std::string_view str);

	/**
	 * 模板的第二个参数表示指定的删除器的类型，删除器是一个可调用对象
	 * 删除器的类型是函数指针类型
	 * 由于函数不能被传递，而函数指针可以，所以第二个参数需要的就是函数指针类型
	 */
	std::unique_ptr<RSA, decltype(&RSA_free)> m_publicKey;
	std::unique_ptr<RSA, decltype(&RSA_free)> m_privateKey;
};
