#pragma once
#include <string>
#include "Codec.hpp"
#include "Message.pb.h"
  
struct ResponseInformation
{
    bool status;
    std::string secretKeyId;
    std::string clientId;
    std::string serverId;
    std::string data;
};

class ResponseCodec :
    public Codec<ResponseInformation>
{
public:
    ResponseCodec() = default;
    ResponseCodec(const std::string& encodedStr);
    ResponseCodec(const ResponseInformation* respInfo);
    ~ResponseCodec() = default;

    void initFromEncodedString(const std::string& encodedStr);
    void initFromResponseInformation(const ResponseInformation* respInfo);
    std::string encodeInformation() override;
    std::optional<ResponseInformation> decodeString() override;
private:
    std::string m_encodedStr;
    ResponseMessage m_respMsg;
};
