#pragma once
#include <string>
#include <optional>

template <typename T>
class Codec
{
public:
	Codec() = default;
	virtual ~Codec() = default;

	virtual std::string encodeInformation();	// 将结构体信息序列化成字符串
	virtual std::optional<T> decodeString();	// 将字符串反序列化成相应的结构体信息
};

template <typename T>
std::string Codec<T>::encodeInformation()
{
	return std::string();
}

template <typename T>
std::optional<T> Codec<T>::decodeString()
{
	return std::nullopt;	
}