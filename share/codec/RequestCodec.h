#pragma once
#include <string.h>
#include "Codec.hpp"
#include "Message.pb.h"
  
struct RequestInformation
{
    int cmdType;
    std::string serverId;
    std::string clientId;
    std::string data;
    std::string sign;
};

class RequestCodec:
    public Codec<RequestInformation>
{
public:
    RequestCodec() = default;
    RequestCodec(const std::string& encodedStr);        // 传入已经序列化的字符串   
    RequestCodec(const RequestInformation* reqInfo);    // 传入序列化前的结构体对象指针
    ~RequestCodec() = default;

    void initFromEncodedString(const std::string& encodedStr);
    void initFromRequestInformation(const RequestInformation* reqInfo);
    std::string encodeInformation() override;
    std::optional<RequestInformation> decodeString() override;

private:
    std::string m_encodedStr;
    RequestMessage m_reqMsg;   
};
