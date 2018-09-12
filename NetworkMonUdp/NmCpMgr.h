#ifndef _NETWORK_MONITOR_CP_MGR_H
#define _NETWORK_MONITOR_CP_MGR_H
#include "Gess.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;

typedef struct stUdpIpPort
{
	int nUdpPort;
	char sPeerIp[32];
} UDP_IP_PORT, *PUDP_IP_PORT;

#define UDP_IP_PORT_MAX_NUM 8
typedef struct stUdpSvr
{
	unsigned int uiSeqNoReq;
	unsigned int uiSeqNoRsp;
	unsigned int uiSendNum;
	//
	unsigned int uiCloseCount;
	unsigned int uiLossCount;
	unsigned int uiSeqErrCount;
	unsigned int uiDelayMin;
	unsigned int uiDelayMax;
	unsigned int uiDelayAvg;
	unsigned int uiDelayTotal;
	unsigned int uiDelayCount;
	int	nTimeDiff;
	//
	int nCount;
	int nIdx;
	UDP_IP_PORT oSvr[UDP_IP_PORT_MAX_NUM];

	void SwitchIdx()
	{
		nIdx++;
		if (nIdx >= min(nCount, UDP_IP_PORT_MAX_NUM))
		{
			nIdx = 0;
		}
	};
} UDP_SVR_INFO, *PUDP_SVR_INFO;

#define MS_MAGIC_NUMBER		0x1321A85
typedef enum
{
	MSCMD_HELLO_REQ = 10000,
	MSCMD_HELLO_RSP,
	MSCMD_CMD_UNKOWN
} EMSCMD_CMD;

typedef struct stPkt
{
	unsigned int uiLen;
	unsigned int uiMagicNum;
	unsigned int uiCmdID;
	unsigned int uiSeqNo;
	unsigned int uiKey1;
	unsigned int uiKey2;
	int			 uisTime;
	int          nTickReq;
	int          nTickRsp;
} MS_PKT, *PMS_PKT;

class CConfigImpl;
class CNmCpMgr
{
public:
	CNmCpMgr();
	virtual ~CNmCpMgr();

	int Init();
	void Finish();
	int Start();
	void Stop();
	int Run();
private:
	void ActiveHello();
	void PassiveHello(MS_PKT& oPktReq, struct sockaddr_in& addr);
	void OnUdpPkt(const char* pBuf, int nLen, struct sockaddr_in& addr);
	void OnHelloRsp(MS_PKT& oPkt);
	int NowTimeStamp();
	int MSecFromTimestamp(int nTimeStamp);
	int NowMSec();

	std::string				m_sProcName;			//½ø³ÌÃû³Æ
	int                     m_nEndThread;
	CConfigImpl*			m_pConfig;
	tsocket_t				m_sLcl;	
	int						m_nUdpPortLcl;
	string                  m_sIpMe;
	vector<UDP_SVR_INFO>	m_vUdpSvr;
	size_t                  m_nUdpSvrNum;
	unsigned int			m_uiCount;
};
#endif