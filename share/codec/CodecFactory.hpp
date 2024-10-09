#pragma once
#include <memory>
#include "Codec.hpp"

template <typename T>
class CodecFactory
{ 
public:
	CodecFactory() = default;
    virtual ~CodecFactory() = default;

	virtual std::unique_ptr<Codec<T>> createCodec() = 0;
};