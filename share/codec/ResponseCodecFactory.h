#pragma once

#include "CodecFactory.hpp"
#include "ResponseCodec.h"
 
class ResponseCodecFactory :
    public CodecFactory<ResponseInformation>
{ 
public:
    ResponseCodecFactory(const std::string& encodedStr);
    ResponseCodecFactory(const ResponseInformation* respInfo);
    ~ResponseCodecFactory() = default;

    std::unique_ptr<Codec<ResponseInformation>> createCodec() override;
private:
    bool m_isEncodedStr;
    std::string m_encodedStr;
    const ResponseInformation* m_respInfo;
};
