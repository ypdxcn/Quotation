#ifndef _NM_SERVICE_H
#define _NM_SERVICE_H
#include "WorkThread.h"

#define	PKT_TS_REQ				0x00000001
#define	PKT_TS_RSP				0x10000001
typedef struct stReq
{
	unsigned int uicTickCount1;
	unsigned int uicTickCount2;
	unsigned int uisTime;
} TIMESTAMP_BODY, *PTIMESTAMP_BODY;

typedef struct
{
	unsigned int uiCloseCount;
	unsigned int uiDelayMin;
	unsigned int uiDelayMax;
	unsigned int uiDelayAvg;
	unsigned int uiDelayTotal;
	unsigned int uiDelayCount;
	int	nTimeDiff;
	char sIp[32];
} TS_STATICS;

class CNmCpMgr;
class CBinBlockPkt;
class CNmService : public CWorkThread
{
public:
	CNmService(CNmCpMgr* pParent);
	~CNmService(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int HandleMoncPkt(CBinBlockPkt& oPkt, unsigned long ulKey);
	int HandleMonsPkt(CBinBlockPkt& oPkt);
	int HandleConnect(unsigned long ulKey, const string& sIp);
	int HandleClose(unsigned long ulKey);
private:
	CConfig*		m_pCfg;
	CNmCpMgr*		m_pParent;

	typedef struct
	{
		unsigned long ulKey;
		TIMESTAMP_BODY tsBody;
	} TS_PKT;

	unsigned int	m_uiKeyPrefix;
	int				m_nState[3];
	TS_STATICS		m_stTsStatic[3];
	vector<TS_PKT>	m_vPkt;
	CGessMutex		m_csPkt;
private:
	int ThreadEntry();
	int End();

	int ToMoncxReq(unsigned long ulKey);
	unsigned int NowTimeStamp();
	unsigned int NowMSec();
	unsigned int MSecFromTimestamp(unsigned int uiTimeStamp);
};
#endif