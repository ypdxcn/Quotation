#ifndef _TRANSLATOR_CP_H
#define _TRANSLATOR_CP_H
#include "Comm.h"
#include "workthread.h"
#include "BroadcastPacket.h"
#include "SamplerPacket.h"
#include "hsstruct.h"

struct SendUDPDataItem
{
	char				 m_szFlag[4];					// 标志
	CodeInfo			 m_ciStockCode;					// 代码
	char				 m_cStockName[STOCK_NAME_SIZE]; // 名称

	int					 m_lDataLen;

	long				 m_lPreClose; // 昨收盘

	union
	{
		struct
		{
			ShareRealTimeData	 m_sData;						// 指向 ShareRealTimeData 中任意一个
			ETFStockNowData      m_sETF;
		};

		char				 m_cData[1];
	};

	SendUDPDataItem(int lDataLen = 0)
	{
		memset(this,0,sizeof(SendUDPDataItem));
		m_lDataLen = lDataLen;
		SetFlag();
	}

	void Empty()
	{
		m_lPreClose = 0;
	}

	void SetFlag(const char* strFlag = "HSHS")
	{
		strncpy(m_szFlag,strFlag,sizeof(m_szFlag));
	}

	BOOL IsValid(int nLen)
	{
		if( nLen < 32 )
			return 0;

		return ( m_szFlag[0] == 'H' &&
			m_szFlag[1] == 'S' &&
			m_szFlag[2] == 'H' &&
			m_szFlag[3] == 'S' );
	}

	char* GetName(char* szRet)
	{
		strncpy(szRet,m_cStockName,sizeof(m_cStockName));
		//YlsAllTrim(szRet);
		return szRet;
	}

};

#define IS_CURR_GOLD(CODE,CODETYPE)( (!strcmp(CODE, "AG999") || !strncmp(CODE, "AG9999",6) ||!strncmp(CODE, "AU100",5) || !strncmp(CODE, "AU50",4) ||!strncmp(CODE, "AU9995",6) || !strncmp(CODE, "AU9999",6) || !strncmp(CODE, "PT9995",6) || (MakeMainMarket(CODETYPE) == (HJ_MARKET | HJ_WORLD)) ))

#pragma pack()

class CSamplerCpMgr;
class CTranslator :public CConnectPointAsyn, public CWorkThread
{
public:
	CTranslator(void);
	~CTranslator(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);


	unsigned int QueueLen() {return m_deqQuotation.size();}	
	unsigned int InPktStatic() {return m_uiInCount;}
private:
	CSamplerCpMgr*     m_pSamplerCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;

	std::deque<CSamplerPacket> m_deqQuotation;
	CCondMutex	m_deqCondMutex;

	///储存黄金行情
	map<std::string, QUOTATION*> m_mapQuotation;

	int ThreadEntry();
	int End();
	
	typedef struct tagStructUdpInfo
	{
		string sIp;
		unsigned short usPort;
		tsocket_t sSocketSendUdp;
		struct sockaddr_in stAddr;

		tagStructUdpInfo()
		{
			sIp = "";
			usPort = 0;
			sSocketSendUdp = INVALID_SOCKET;
			memset(&stAddr, 0x00,sizeof(stAddr));
		}
	} UDP_INFO, *PUDP_INFO;
	vector<UDP_INFO> m_vUdpInfo;
	unsigned int m_uiFwdCount;
	//接收包数量
	unsigned int m_uiInCount;		

	int Translate(CSamplerPacket& oPktSrc);
	int SendUdpPkt(QUOTATION& stQuotation);
	void TranslateCode(char* pcDestCode, const char* pccOriCode);
};
#endif