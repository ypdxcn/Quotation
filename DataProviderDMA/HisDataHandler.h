#ifndef _HIS_DATA_HANDLER_H
#define _HIS_DATA_HANDLER_H

#include <fstream>
#include <iostream>
#include "Workthread.h"
#include "GessDate.h"
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

	void SwitchTradeDate(unsigned int uiDate);
private:	
	void WriteTick(const string& sQuotation);
	void OpenHisdataFile(unsigned int uiDate);
private:
	int ThreadEntry();
	int End();
private:
	CConfig*				m_pCfg;
	vector<string>			m_vTickInst;	
	map<string, ofstream*>	m_mapOfsTick;

	CGessDate				m_oDateLast;
	string					m_sFilePathAbs;

	std::deque<string>		m_deqPktVal;
	CCondMutex				m_deqCondMutex;

	unsigned int			m_uiDate;
};

#endif