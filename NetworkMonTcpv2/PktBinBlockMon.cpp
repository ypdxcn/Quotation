#include "PktBinBlockMon.h"
#include "strutils.h"

using namespace strutils;

CBinBlockPkt::CBinBlockPkt(unsigned int uiCmdID) 
:m_uiCmdID(uiCmdID)
,m_uiKeyID(0)
,m_uiBufLen(0)
,m_pcBuf(0)
{
	m_sCmdID = strutils::ToString<unsigned int>(uiCmdID);
}

CBinBlockPkt::CBinBlockPkt()
:m_uiCmdID(0xFFFFFFFF)
,m_uiKeyID(0)
,m_uiBufLen(0)
,m_pcBuf(0)
,m_sCmdID("")
{
}

CBinBlockPkt::CBinBlockPkt(const CBinBlockPkt& pkt)
{
	const BIN_BLOCK_HEADER* pHeader = (BIN_BLOCK_HEADER*)(pkt.m_pcBuf);		
	if (0 != pHeader)
	{
		m_uiBufLen = pkt.m_uiBufLen;;
		m_uiCmdID = pHeader->uiCmdID;
		m_sCmdID = strutils::ToString<unsigned int>(m_uiCmdID);
		m_uiKeyID = pHeader->uiKeyID;

		m_pcBuf = new char[m_uiBufLen + sizeof(BIN_BLOCK_HEADER)];
		memcpy(m_pcBuf, pkt.m_pcBuf, m_uiBufLen + sizeof(BIN_BLOCK_HEADER));
	}
}

CBinBlockPkt& CBinBlockPkt::operator=(const CBinBlockPkt& pkt)
{
	const BIN_BLOCK_HEADER* pHeader = (BIN_BLOCK_HEADER*)(pkt.m_pcBuf);		
	if (0 != pHeader)
	{
		m_uiBufLen = pkt.m_uiBufLen;;
		m_uiCmdID = pHeader->uiCmdID;
		m_sCmdID = strutils::ToString<unsigned int>(m_uiCmdID);		
		m_uiKeyID = pHeader->uiKeyID;

		m_pcBuf = new char[m_uiBufLen + sizeof(BIN_BLOCK_HEADER)];
		memcpy(m_pcBuf, pkt.m_pcBuf, m_uiBufLen + sizeof(BIN_BLOCK_HEADER));
	}
	return *this;
}

CBinBlockPkt::~CBinBlockPkt() 
{
	if (0 != m_pcBuf)
	{
		delete []m_pcBuf;
		m_pcBuf = 0;
		m_uiBufLen = 0;
	}
}

const char* CBinBlockPkt::Encode(unsigned int& uiLen)
{
	if (0 == m_uiBufLen)
	{
		uiLen = 0;
		return 0;
	}

	uiLen = m_uiBufLen + sizeof(BIN_BLOCK_HEADER);
	return m_pcBuf;
}

void CBinBlockPkt::Decode(const char * pData, unsigned int nLen)
{
	if (0 != m_pcBuf)
	{
		delete []m_pcBuf;
		m_pcBuf = 0;
	}

	BIN_BLOCK_HEADER* pHeader = (BIN_BLOCK_HEADER*)(pData);
	pHeader->uiLen = htonl(pHeader->uiLen);
	unsigned int uiHeadLen = sizeof(BIN_BLOCK_HEADER);
	if (0 != pHeader)
	{
		if (pHeader->uiLen == nLen)
		{
			m_uiBufLen = pHeader->uiLen - uiHeadLen;
			m_uiCmdID = pHeader->uiCmdID;
			m_sCmdID = strutils::ToString<unsigned int>(m_uiCmdID);	
			m_uiKeyID = pHeader->uiKeyID;
			m_pcBuf = new char[nLen];
			memcpy(m_pcBuf, pData, nLen);
		}
	}
}

unsigned int CBinBlockPkt::BufLength() 
{
	return m_uiBufLen;
}

const char* CBinBlockPkt::Buffer() 
{
	if (0 == m_pcBuf)
		return 0;

	return m_pcBuf + sizeof(BIN_BLOCK_HEADER);
}

void CBinBlockPkt::FillBuf(char* pData, unsigned int nLen)
{
	if (0 == pData || 0 == nLen)
		return;

	m_pcBuf = new char[nLen + sizeof(BIN_BLOCK_HEADER)];
	memcpy(m_pcBuf + sizeof(BIN_BLOCK_HEADER), pData, nLen);
	m_uiBufLen = nLen;

	BIN_BLOCK_HEADER* pHead = (BIN_BLOCK_HEADER*)m_pcBuf;
	pHead->uiCmdID = m_uiCmdID;
	pHead->uiKeyID = m_uiKeyID;
	pHead->uiLen = htonl(nLen + sizeof(BIN_BLOCK_HEADER));
}

void CBinBlockPkt::FillBuf(unsigned int uiCmdID, char* pData, unsigned int nLen)
{
	m_uiCmdID = uiCmdID;
	m_sCmdID = strutils::ToString<unsigned int>(m_uiCmdID);	
	return FillBuf(pData, nLen);
}

unsigned int CBinBlockPkt::CmdID() 
{
	return m_uiCmdID;
}

void CBinBlockPkt::SetCmdID(unsigned int uiCmdID) 
{
	m_uiCmdID = uiCmdID;
	m_sCmdID = strutils::ToString<unsigned int>(m_uiCmdID);	
}

std::string CBinBlockPkt::RouteKey()
{
	return strutils::ToString<unsigned int>(m_uiKeyID);
}

void CBinBlockPkt::RouteKey(unsigned int uiKeyID)
{
	m_uiKeyID = uiKeyID;
}

void CBinBlockPkt::CopyKey(const CBinBlockPkt& pkt)
{
	m_uiKeyID = pkt.m_uiKeyID;
}