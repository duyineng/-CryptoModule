#include "RequestCodec.h"
#include "../../share/SimpleLogger.h"

 
RequestCodec::RequestCodec(std::string encodedStr)
	: m_encodedStr(encodedStr)
{
}

RequestCodec::RequestCodec(const RequestInformation* reqInfo)
{
	m_reqMsg.set_cmdtype(reqInfo->cmdType);
	m_reqMsg.set_clientid(reqInfo->clientId);
	m_reqMsg.set_serverid(reqInfo->serverId);
	m_reqMsg.set_data(reqInfo->data);
	m_reqMsg.set_sign(reqInfo->sign);
}

void RequestCodec::initFromEncodedString(std::string encodedStr)
{
	m_encodedStr = encodedStr;
}

void RequestCodec::initFromRequestInformation(const RequestInformation* reqInfo)
{
	m_reqMsg.set_cmdtype(reqInfo->cmdType);
	m_reqMsg.set_clientid(reqInfo->clientId);
	m_reqMsg.set_serverid(reqInfo->serverId);
	m_reqMsg.set_data(reqInfo->data);
	m_reqMsg.set_sign(reqInfo->sign);
}

std::string RequestCodec::encodeInformation()
{
	std::string output;
	if(!m_reqMsg.SerializeToString(&output))
	{
		LOG_ERROR("Failed to serialize RequestMessage instance");
		return std::string();
	}

	return output;
}

std::optional<RequestInformation> RequestCodec::decodeString()
{
	if(!m_reqMsg.ParseFromString(m_encodedStr))
	{
		LOG_ERROR("Failed to parse encoded string");
		return std::nullopt;
	}

	RequestInformation reqInfo;
	reqInfo.cmdType = m_reqMsg.cmdtype();
	reqInfo.clientId = m_reqMsg.clientid();
	reqInfo.serverId = m_reqMsg.serverid();
	reqInfo.data = m_reqMsg.data();
	reqInfo.sign = m_reqMsg.sign();

	return reqInfo;
}
template class Codec<RequestInformation>;