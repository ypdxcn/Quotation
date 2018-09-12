#ifndef _HIS_DATA_HANDLER_H
#define _HIS_DATA_HANDLER_H

#include <fstream>
#include <iostream>
#include "Workthread.h"
#include "BroadcastPacket.h"
#include "SamplerPacket.h"


class CProviderCpMgr;
class CHisDataHandler : public CWorkThread
{
public:
	CHisDataHandler();
	~CHisDataHandler(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int Enque(const string& sQuotation);

	string HandleCmdLine(const string& sCmd, const vector<string>& vecPara);
private:
	CConfig*		m_pCfg;
	ofstream		m_ofsQuotation;
	int				m_nLogFlag;

	std::deque<string> m_deqPktVal;
	CCondMutex	m_deqCondMutex;

	int ThreadEntry();
	int End();

	//
	string HandleCmdLineLog(const string& sCmd, const vector<string>& vecPara);
};

#endif