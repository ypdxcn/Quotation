#include "HisDataHandler.h"
#include "ProviderCpMgr.h"
#include "GessDateTime.h"

CHisDataHandler::CHisDataHandler()
:m_pCfg(0)
,m_nLogFlag(0)
{
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

	string sVal;
	if (0 == pCfg->GetProperty("RECORD",sVal))
	{
		m_nLogFlag = FromString<int>(sVal);
	}

	if (1 == m_nLogFlag)
	{
		string sFileName = CGessDate::NowToString("");
		sFileName += CGessTime::NowToString();
		sFileName += ".dat";
		sFileName = ".\\hisdata\\" + sFileName;
		m_ofsQuotation.open(sFileName.c_str(),ios::binary | ios::out | ios::trunc);
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

	if (m_ofsQuotation.is_open())
	{
		m_ofsQuotation.close();
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
				if (1 == m_nLogFlag)
				{
					m_ofsQuotation.write((const char*)sQuotationVal.data(),sQuotationVal.length());
					m_ofsQuotation.flush();
				}
			}
			catch(...)
			{
				CRLog(E_ERROR,"%s","Unknown exception!");
			}

			m_deqCondMutex.Lock();
			m_deqPktVal.pop_front();
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

int CHisDataHandler::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}

string CHisDataHandler::HandleCmdLine(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Provider->\r\n";
	if (sCmd == "log" || sCmd == "l")
	{
		return HandleCmdLineLog(sCmd, vecPara);
	}
	else
	{
		return sRtn;
	}
}

string CHisDataHandler::HandleCmdLineLog(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Provider->\r\n";
	if (vecPara.size() == 0)
	{
		if (1 == m_nLogFlag)
		{
			sRtn += "now is on!";
		}
		else
		{
			sRtn += "now is off!";
		}
		return sRtn;
	}

	string sPara = vecPara[0];
	if (sPara == "on")
	{
		if (!m_ofsQuotation.is_open())
		{
			string sFileName = CGessDate::NowToString("");
			sFileName += CGessTime::NowToString();
			sFileName += ".dat";
			sFileName = ".\\hisdata\\" + sFileName;
			m_ofsQuotation.open(sFileName.c_str(),ios::binary | ios::out | ios::trunc);
		}

		m_nLogFlag = 1;
		sRtn += "on!";
	}
	else
	{
		m_nLogFlag = 0;
		if (m_ofsQuotation.is_open())
		{
			m_ofsQuotation.flush();
			m_ofsQuotation.close();
		}
		sRtn += "off!";
	}

	return sRtn;
}
