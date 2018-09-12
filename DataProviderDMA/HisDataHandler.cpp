#include "HisDataHandler.h"
#include "ProviderCpMgr.h"
#include "GessDateTime.h"

CHisDataHandler::CHisDataHandler()
:m_pCfg(0)
,m_sFilePathAbs("")
{
	m_uiDate = CGessDate::ThisYear()*10000 + CGessDate::ThisMonth()*100 + CGessDate::ThisDay();
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

	char szPathBuf[2048];
	getcwd(szPathBuf,sizeof(szPathBuf));
	m_sFilePathAbs = szPathBuf;
	m_sFilePathAbs += PATH_SLASH;
	m_sFilePathAbs += "hisdata";
	m_sFilePathAbs += PATH_SLASH;

	string sValue;
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
}

//清理资源
void CHisDataHandler::Finish()
{	
	m_deqPktVal.clear();
}

int CHisDataHandler::Enque(const string& sQuotation)
{
	try
	{
		m_deqCondMutex.Lock();
		m_deqPktVal.push_back(sQuotation);
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
			while(m_deqPktVal.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			string& sQuotationVal = m_deqPktVal.front();			
			m_deqCondMutex.Unlock();

			try
			{
				WriteTick(sQuotationVal);
			}
			catch(...)
			{
				CRLog(E_ERROR,"%s","Unknown exception!");
			}

			m_deqCondMutex.Lock();
			m_deqPktVal.pop_front();
			m_deqCondMutex.Unlock();
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

void CHisDataHandler::WriteTick(const string& sQuotation)
{
	try
	{
		const char* pData = sQuotation.data();
		const char* pTmp = pData + 7;
		unsigned char ucLen = *pTmp;
		char szInstID[128] = {0};
		memset(szInstID, 0x00, ucLen + 1);
		memcpy(szInstID, pTmp + 1, ucLen);

		string sInstID = szInstID;
		map<string, ofstream*>::iterator it = m_mapOfsTick.find(sInstID);
		if (it != m_mapOfsTick.end())
		{
			ofstream* ofsTick = (*it).second;
			if (0 != ofsTick)
			{
				SYSTEMTIME st;
				::GetLocalTime(&st);
				unsigned int uiTime = st.wHour*10000000 + st.wMinute*100000 + st.wSecond*1000 + st.wMilliseconds;
				ofsTick->write((const char*)(&uiTime), sizeof(uiTime));
				ofsTick->write((const char*)(sQuotation.data()), sQuotation.length());
				ofsTick->flush();
			}
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

void CHisDataHandler::SwitchTradeDate(unsigned int uiDate)
{
	OpenHisdataFile(uiDate);
}

void CHisDataHandler::OpenHisdataFile(unsigned int uiDate)
{
	try
	{
		string sInst;
		for (size_t nIdx = 0; nIdx < m_vTickInst.size(); nIdx++)
		{
			string sInst = m_vTickInst[nIdx];
			string sTickFileName = m_sFilePathAbs + "250ms_" + sInst + "_";
			sTickFileName += strutils::ToString<unsigned int>(uiDate);
			sTickFileName += ".dat";

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
			ofs->open(sTickFileName.c_str(),ios::binary | ios::out | ios::app);
		}
	}
	catch(...)
	{
		CRLog(E_CRITICAL, "%s", "unknown exceptin!");
	}
}
