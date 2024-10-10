#include "ResponseCodecFactory.h"
 
ResponseCodecFactory::ResponseCodecFactory(const std::string& encodedStr)
{
	m_encodedStr = encodedStr;
	m_isEncodedStr = true;
}

ResponseCodecFactory::ResponseCodecFactory(const ResponseInformation* respInfo)
{
	m_respInfo = respInfo;
	m_isEncodedStr = false;
}

std::unique_ptr<Codec<ResponseInformation>> ResponseCodecFactory::createCodec()
{
	if (m_isEncodedStr == true)
	{
		return std::make_unique<ResponseCodec>(m_encodedStr);
	}

	return std::make_unique<ResponseCodec>(m_respInfo);	
}
