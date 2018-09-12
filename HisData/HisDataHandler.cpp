#include "HisDataHandler.h"
#include "HisDataCpMgr.h"
#include "GessDateTime.h"

CHisDataHandler::CHisDataHandler(CHisDataCpMgr* p)
:m_pProviderCpMgr(p)
,m_pCfg(0)
,m_nTest(0)
,m_uiPkts(0)
{
	m_sFilePathAbs = "";
}

CHisDataHandler::~CHisDataHandler(void)
{
}

//初始化配置，创建线程池
int CHisDataHandler::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	//CConfig* pConfig = m_pCfg->GetCfgGlobal();

	string sValue;
	if (0 == pCfg->GetProperty("TEST",sValue))
	{
		m_nTest = FromString<int>(sValue);
	}

	char szPathBuf[2048];
	getcwd(szPathBuf,sizeof(szPathBuf));
	m_sFilePathAbs = szPathBuf;
	m_sFilePathAbs += PATH_SLASH;
	m_sFilePathAbs += "hisdata";
	m_sFilePathAbs += PATH_SLASH;

	sValue.clear();
	if (0 == pCfg->GetProperty("PATH", sValue))
	{
		bool blFlag = false;
		string::size_type iPos = 0;
		for (iPos = sValue.length() - 1; iPos >= 0; iPos--)
		{
			if (blFlag)
			{
				if (sValue[iPos] != '/' && sValue[iPos] != '\\')
				{
					break;
				}
			}
			else
			{
				if (sValue[iPos] == '/' || sValue[iPos] == '\\')
				{
					blFlag = true;
				}
				else
				{
					break;
				}
			}
		}

		if (iPos >= 0)
		{
			m_sFilePathAbs = sValue.substr(0,iPos + 1) + PATH_SLASH;
		}
	}

	if (0 == pCfg->GetProperty("INST_TICK", m_vTickInst))
	{

	}
	return 0;
}


//启动调度线程及工作线程池
int CHisDataHandler::Start()
{
	//启动调度线程
	BeginThread();

	OpenHisdataFile();
	return 0;
}

//停止工作线程池及调度线程
void CHisDataHandler::Stop()
{
	//停止调度线程
	CRLog(E_APPINFO,"%s","Stop HisDataHandler Thread");
	EndThread();

	ofstream* ofs = 0;
	map<string, ofstream*>::iterator it;
	for (it = m_mapOfsTick.begin(); it != m_mapOfsTick.end(); ++it)
	{
		ofs = (*it).second;
		ofs->close();
		delete ofs;
	}

	if (1 == m_nTest)
	{
		if (m_ofsQuotation.is_open())
			m_ofsQuotation.close();
	}
}

//清理资源
void CHisDataHandler::Finish()
{	
	m_deqQuotation.clear();
}

int CHisDataHandler::Enque(QUOTATION& stQuotation)
{
	try
	{
		m_deqCondMutex.Lock();
		m_deqQuotation.push_back(stQuotation);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		return 0;
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"%s","packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

int CHisDataHandler::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqQuotation.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			QUOTATION& stQuotation = m_deqQuotation.front();			
			m_deqCondMutex.Unlock();

			try
			{
				m_uiPkts++;
				if (m_uiPkts % 10000 == 0)
					CRLog(E_STATICS, "Handled pkts:%u", m_uiPkts);

				if (1 == m_nTest && m_ofsQuotation.is_open())
				{
					if (!m_oDateLast.IsToday())
					{
						OpenHisdataFile();
						m_oDateLast.Day2Today();
					}

					m_ofsQuotation.write((const char*)(&stQuotation),sizeof(QUOTATION));
					m_ofsQuotation.flush();
				}
				

				string sInstID;
				sInstID.assign(stQuotation.m_CodeInfo.m_acCode, sizeof(stQuotation.m_CodeInfo.m_acCode));
				map<std::string, QUOTATION>::iterator it = m_mapQuotation.find(sInstID);
				if (it != m_mapQuotation.end())
				{
					if (stQuotation.m_uiVolume != (*it).second.m_uiVolume 
					|| stQuotation.m_uiTurnOver != (*it).second.m_uiTurnOver 
					|| stQuotation.m_uiLast != (*it).second.m_uiLast)
					{
						WriteTick( stQuotation);
						m_mapQuotation[sInstID] = stQuotation;						
					}
				}
				else
				{
					WriteTick(stQuotation);
					m_mapQuotation.insert(pair<string,QUOTATION>(sInstID, stQuotation));				
				}
			}
			catch(...)
			{
				CRLog(E_ERROR,"%s","Unknown exception!");
			}

			m_deqCondMutex.Lock();
			m_deqQuotation.pop_front();
			m_deqCondMutex.Unlock();

			//	//存盘处理
			//	//......
		}
		CRLog(E_APPINFO,"%s","RiskHandler Thread exit!");
		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

void CHisDataHandler::WriteTick(const QUOTATION& stQuotation)
{
	try
	{
		if (!m_oDateLast.IsToday())
		{
			OpenHisdataFile();
			m_oDateLast.Day2Today();
		}

		string sInstID = stQuotation.m_CodeInfo.m_acCode;
		map<string, ofstream*>::iterator it = m_mapOfsTick.find(sInstID);
		if (it != m_mapOfsTick.end())
		{
			ofstream* ofsTick = (*it).second;
			ofsTick->write((const char*)(&stQuotation.m_uiDate), sizeof(stQuotation.m_uiDate));
			ofsTick->write((const char*)(&stQuotation.m_uiTime), sizeof(stQuotation.m_uiTime));
			ofsTick->write((const char*)(&stQuotation.m_CodeInfo.m_usMarketType), sizeof(stQuotation.m_CodeInfo.m_usMarketType));
			ofsTick->write((const char*)(&stQuotation.m_uiLast), sizeof(stQuotation.m_uiLast));

			unsigned int uiMarketType = stQuotation.m_CodeInfo.m_usMarketType;
			uiMarketType &= 0xFFFF;
			if (HJ_MARKET|HJ_SH_CURR == uiMarketType)
			{//SGE
				ofsTick->write((const char*)(&stQuotation.m_uiVolume), sizeof(stQuotation.m_uiVolume));
				ofsTick->write((const char*)(&stQuotation.m_uiHigh), sizeof(stQuotation.m_uiHigh));
				ofsTick->write((const char*)(&stQuotation.m_uiLow), sizeof(stQuotation.m_uiLow));
				ofsTick->write((const char*)(&stQuotation.m_uiChiCangLiang), sizeof(stQuotation.m_uiChiCangLiang));						
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiVol), sizeof(stQuotation.m_Ask[0].m_uiVol));						
				ofsTick->write((const char*)(&stQuotation.m_Ask[1].m_uiPrice), sizeof(stQuotation.m_Ask[1].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Ask[1].m_uiVol), sizeof(stQuotation.m_Ask[1].m_uiVol));	
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiVol), sizeof(stQuotation.m_Bid[0].m_uiVol));						
				ofsTick->write((const char*)(&stQuotation.m_Bid[1].m_uiPrice), sizeof(stQuotation.m_Bid[1].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[1].m_uiVol), sizeof(stQuotation.m_Bid[1].m_uiVol));	
			}
			else if (HJ_MARKET|HJ_WORLD == uiMarketType)
			{//伦敦金
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));					
			}
			else if (HJ_MARKET|HJ_OTHER == uiMarketType)
			{//其他黄金				
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));					
			}
			else if (WP_MARKET|WP_INDEX == uiMarketType)
			{//外盘指数				
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));					
			}
			else if (FOREIGN_MARKET | WH_BASE_RATE == uiMarketType ||
				FOREIGN_MARKET | WH_ACROSS_RATE == uiMarketType)
			{//外汇
				
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));
			}
			else if (WP_MARKET | WP_NYMEX == uiMarketType)
			{//NYMEX 原油期货				
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiVol), sizeof(stQuotation.m_Ask[0].m_uiVol));						
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiVol), sizeof(stQuotation.m_Bid[0].m_uiVol));						
			}
			else if (WP_MARKET | WP_COMEX == uiMarketType)
			{//COMEX 黄金期货
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiVol), sizeof(stQuotation.m_Ask[0].m_uiVol));						
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiVol), sizeof(stQuotation.m_Bid[0].m_uiVol));						
			}
			else
			{
				ofsTick->write((const char*)(&stQuotation.m_Ask[0].m_uiPrice), sizeof(stQuotation.m_Ask[0].m_uiPrice));					
				ofsTick->write((const char*)(&stQuotation.m_Bid[0].m_uiPrice), sizeof(stQuotation.m_Bid[0].m_uiPrice));					
			}
			ofsTick->flush();
		}
	}
	catch(...)
	{

	}
}

int CHisDataHandler::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}

void CHisDataHandler::OpenHisdataFile()
{
	try
	{
		string sTmp = CGessDate::NowToString("");
		sTmp += "_";
		sTmp += CGessTime::NowToString();

		string sInst;
		for (size_t nIdx = 0; nIdx < m_vTickInst.size(); nIdx++)
		{
			string sInst = m_vTickInst[nIdx];
			string sTickFileName = sTmp + ".dat";
			sTickFileName = m_sFilePathAbs + "Tick_" + sInst + "_" + sTickFileName;

			ofstream* ofs = 0;
			map<string, ofstream*>::iterator it = m_mapOfsTick.find(sInst);
			if (it != m_mapOfsTick.end())
			{
				ofs = (*it).second;
				if (ofs->is_open())
				{
					ofs->close();
				}
			}
			else
			{
				ofs = new ofstream;
				m_mapOfsTick[sInst] = ofs;
			}
			ofs->open(sTickFileName.c_str(),ios::binary | ios::out | ios::trunc);
		}

		if (1 == m_nTest)
		{
			string sTestFileName = sTmp + ".dat";
			sTestFileName = m_sFilePathAbs + sTestFileName;

			if (m_ofsQuotation.is_open())
			{
				m_ofsQuotation.close();
			}
			m_ofsQuotation.open(sTestFileName.c_str(),ios::binary | ios::out | ios::trunc);
		}
	}
	catch(...)
	{
		CRLog(E_CRITICAL, "%s", "unknown exceptin!");
	}
}

string CHisDataHandler::HandleCmdLine(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "HisDataHandler->\r\n";
	return sRtn;
}