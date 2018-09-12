#include "SamplerPacket.h"

CSamplerPacket::CSamplerPacket(): CMsgBlock(1024, 4)
{
}

CSamplerPacket::CSamplerPacket(unsigned int uiCmdID): CMsgBlock(1024, 4)
{
	m_Msg.SetField(MSG_CMD_ID,uiCmdID);
	m_sCmdID = strutils::ToHexString<unsigned int>(uiCmdID);
}

CSamplerPacket::CSamplerPacket(CMessage& msg, unsigned int uiCmdID): CMsgBlock(msg,1024, 4)
{
	m_Msg.SetField(MSG_CMD_ID,uiCmdID);
	m_sCmdID = strutils::ToHexString<unsigned int>(uiCmdID);
}

CSamplerPacket::~CSamplerPacket(void)
{
}

void  CSamplerPacket::Decode(const char * pData, unsigned int nLength)
{
	//BlockDecode(pData,nLength);
	//GetCmdID();
}

const char* CSamplerPacket::Encode(unsigned int & usLen)
{
	//return BlockEncode(usLen);

	return  "";
}

const string& CSamplerPacket::GetCmdID()
{
	unsigned int uiCmdID = 0; 
	m_Msg.GetField(MSG_CMD_ID, uiCmdID);
	m_sCmdID = strutils::ToHexString<unsigned int>(uiCmdID);
	return m_sCmdID;
}

std::string CSamplerPacket::RouteKey()
{
	unsigned int uiNodeID = 0; 
	m_Msg.GetField(MSG_NODE_ID, uiNodeID);
	return strutils::ToString<unsigned int>(uiNodeID);
}