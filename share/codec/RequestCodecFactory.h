#pragma once
#include "CodecFactory.hpp"
#include "RequestCodec.h"

class RequestCodecFactory :
    public CodecFactory<RequestInformation>
{
public:
    RequestCodecFactory(const std::string& encodedStr);
    RequestCodecFactory(const RequestInformation* reqInfo);
    ~RequestCodecFactory() = default;

    std::unique_ptr<Codec<RequestInformation>> createCodec() override;  // 创建RequestCodec实例
private:
    bool m_isEncodedStr;
    std::string m_encodedStr;
    const RequestInformation* m_reqInfo;
};
