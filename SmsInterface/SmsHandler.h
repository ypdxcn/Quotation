#ifndef _HIS_DATA_HANDLER_H
#define _HIS_DATA_HANDLER_H

#include <fstream>
#include <iostream>
#include "Workthread.h"
#include "BroadcastPacket.h"
#include "GessTime.h"

class CSmsCpMgr;
class CSmsHandler : public CConnectPointAsyn, public CWorkThread
{
public:
	CSmsHandler();
	~CSmsHandler(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();

	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &GessPacket);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);

	string HandleCmdLine(const string& sCmd, const vector<string>& vecPara);
private:
	CSmsCpMgr*     m_pCpMgr;
	CConfig*		m_pCfg;
	string			m_s3wName;
	int				m_n3wPort;
	string			m_sUserID;
	string			m_sPwd;
	string			m_sAdminMobile;
	unsigned int	m_uiSendNum;
	CGessTime		m_oSmsTimeLast;

	typedef std::pair<CGessTime,CGessTime> PairInterval;
	typedef std::map<std::string,std::vector<PairInterval>> MapNotify;
	MapNotify	m_mapNotify;

	std::deque<CBroadcastPacket> m_deqEvt;
	CCondMutex	m_deqCondMutex;

	int ThreadEntry();
	int End();
	void HandlePacket(CBroadcastPacket& pkt);
	void SmsNotify(const string& sSmsContent);
};

#endif