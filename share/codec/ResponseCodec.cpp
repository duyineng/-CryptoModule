#include "ResponseCodec.h"
#include "Message.pb.h"
#include "../../share/SimpleLogger.h"
 
ResponseCodec::ResponseCodec(const std::string& encodedStr)
{
	m_encodedStr = encodedStr;
}

ResponseCodec::ResponseCodec(const ResponseInformation* respInfo)
{
	m_respMsg.set_status(respInfo->status);
	m_respMsg.set_secretkeyid(respInfo->secretKeyId);
	m_respMsg.set_clientid(respInfo->clientId);
	m_respMsg.set_serverid(respInfo->serverId);
	m_respMsg.set_data(respInfo->data);
}

void ResponseCodec::initFromEncodedString(const std::string& encodedStr)
{
	m_encodedStr = encodedStr;
}

void ResponseCodec::initFromResponseInformation(const ResponseInformation* respInfo)
{
	m_respMsg.set_status(respInfo->status);
	m_respMsg.set_secretkeyid(respInfo->secretKeyId);
	m_respMsg.set_clientid(respInfo->clientId);
	m_respMsg.set_serverid(respInfo->serverId);
	m_respMsg.set_data(respInfo->data);
}

std::string ResponseCodec::encodeInformation()
{
	std::string output;
	if(!m_respMsg.SerializeToString(&output))
	{
		LOG_ERROR("Failed to serialize ResponseMessage instance");
		return std::string();
	}

	return output;
}

std::optional<ResponseInformation> ResponseCodec::decodeString()
{
	if(!m_respMsg.ParseFromString(m_encodedStr))
	{
		LOG_ERROR("Failed to parse encoded string");
		return std::nullopt;
	}

	ResponseInformation resqInfo;
	resqInfo.status = m_respMsg.status();
	resqInfo.secretKeyId = m_respMsg.secretkeyid();
	resqInfo.clientId = m_respMsg.clientid();
	resqInfo.serverId = m_respMsg.serverid();
	resqInfo.data = m_respMsg.data();

	return resqInfo;
}
