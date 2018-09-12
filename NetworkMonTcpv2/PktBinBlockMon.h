#ifndef _PKT_BIN_BLOCK_MON_H_
#define _PKT_BIN_BLOCK_MON_H_

#include "string.h"
#include "Comm.h"


typedef struct
{
	unsigned int	uiLen;
	unsigned int	uiCmdID;
	unsigned int	uiKeyID;
} BIN_BLOCK_HEADER;


class CBinBlockPkt: public CPacketRouteable
{
public:
	CBinBlockPkt(unsigned int uiCmdID);
	CBinBlockPkt();
	CBinBlockPkt(const CBinBlockPkt& pkt);
	CBinBlockPkt& operator=(const CBinBlockPkt& pkt);
	~CBinBlockPkt() ;

	std::string RouteKey();
	void RouteKey(unsigned int uiKeyID);
	void CopyKey(const CBinBlockPkt& pkt);

	const char* Encode(unsigned int& uiLen);
	void Decode(const char * pData, unsigned int nLen);
	unsigned int BufLength();
	const char* Buffer();
	void FillBuf(char* pData, unsigned int nLen);
	void FillBuf(unsigned int uiCmdID, char* pData, unsigned int nLen);
	
	unsigned int CmdID();
	void SetCmdID(unsigned int uiCmdID);
	string& GetCmdID() {return m_sCmdID;}
private:
	unsigned int	m_uiCmdID;
	unsigned int	m_uiKeyID;
	unsigned int	m_uiBufLen;
	char*			m_pcBuf;
	string			m_sCmdID;
};
#endif // _MSG_BLOCK_H_
