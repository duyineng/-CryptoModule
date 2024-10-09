#pragma once
#include <string>
#include <optional>

template <typename T>
class Codec
{
public:
	Codec() = default;
	virtual ~Codec() = default;

	virtual std::string encodeInformation();
	virtual std::optional<T> decodeString() ;
};

template <typename T>
std::string Codec<T>::encodeInformation()
{
	return std::string();
}

template <typename T>
std::optional<T> Codec<T>::decodeString()
{
	return std::nullopt;	// 返回一个空的std::optional对象
}