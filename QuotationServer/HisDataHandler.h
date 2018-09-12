#ifndef _HIS_DATA_HANDLER_H
#define _HIS_DATA_HANDLER_H

#include "Workthread.h"
#include "SamplerPacket.h"
#include "MemData.h"

class CQuoSvrCpMgr;
class CHisDataHandler :public CConnectPointAsyn, public CWorkThread
{
public:
	CHisDataHandler(void);
	~CHisDataHandler(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);

private:
	CQuoSvrCpMgr*     m_pCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;

	std::deque<CSamplerPacket> m_deqQuotation;
	CCondMutex	m_deqCondMutex;

	int ThreadEntry();
	int End();

	int SaveQuotation(CSamplerPacket& oPktSrc);
};

#endif