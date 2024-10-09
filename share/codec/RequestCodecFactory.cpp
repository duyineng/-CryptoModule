#include "RequestCodecFactory.h"
 
RequestCodecFactory::RequestCodecFactory(const std::string& encodedStr)
{
	m_encodedStr = encodedStr;
	m_isEncodedStr = true;
}

RequestCodecFactory::RequestCodecFactory(const RequestInformation* reqInfo)
{
	m_reqInfo = reqInfo;
	m_isEncodedStr = false;
}

std::unique_ptr<Codec<RequestInformation>> RequestCodecFactory::createCodec()
{
	if (m_isEncodedStr == true)
	{
		return std::make_unique<RequestCodec>(m_encodedStr);
	}

	return std::make_unique<RequestCodec>(m_reqInfo);
}
