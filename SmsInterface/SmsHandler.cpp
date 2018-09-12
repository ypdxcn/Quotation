#include <cassert>
#include "SmsHandler.h"
#include "SmsCpMgr.h"
#include "GessDateTime.h"

CSmsHandler::CSmsHandler()
:m_pCpMgr(0)
,m_pCfg(0)
,m_uiSendNum(0)
{
	m_s3wName = "service.winic.org";
	m_n3wPort = 80;
	m_sUserID = "quotest";
	m_sPwd = "quotest";
	m_sAdminMobile = "";
}

CSmsHandler::~CSmsHandler(void)
{
}

//初始化配置，创建线程池
int CSmsHandler::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	string sTmp;
	if (0 == m_pCfg->GetProperty("www",sTmp))
	{
		m_s3wName = trim(sTmp);
	}
	
	if (0 == m_pCfg->GetProperty("port",sTmp))
	{
		m_n3wPort = FromString<int>(sTmp);
	}

	if (0 == m_pCfg->GetProperty("id",sTmp))
	{
		m_sUserID = trim(sTmp);
	}

	if (0 == m_pCfg->GetProperty("pwd",sTmp))
	{
		m_sPwd = trim(sTmp);
	}
	
	if (0 == m_pCfg->GetProperty("admin",sTmp))
	{
		m_sAdminMobile = trim(sTmp);
	}

	if (0 == m_pCfg->GetProperty("notify",sTmp))
	{
		vector<string> vMobile = strutils::explodeQuoted(",", trim(sTmp));
		for (vector<string>::iterator itMobile = vMobile.begin(); itMobile != vMobile.end(); ++itMobile)
		{
			string sMobile = trim(*itMobile);
			if (sMobile == "")
				continue;

			if (0 == m_pCfg->GetProperty(sMobile,sTmp))
			{
				vector<PairInterval> vTimeInterval;
				vTimeInterval.clear();

				vector<string> vTimes = strutils::explodeQuoted(",", trim(sTmp));
				for (vector<string>::iterator itTimePair = vTimes.begin(); itTimePair != vTimes.end(); ++itTimePair)
				{
					vector<string> vTime = strutils::explodeQuoted("-", trim(*itTimePair));
					if (vTime.size() != 2)
						continue;

					if (trim(vTime[0]) == "" || trim(vTime[1]) == "")
						continue;

					CGessTime oBegin, oEnd;
					if (!oBegin.FromString(trim(vTime[0])))
						continue;

					if (!oEnd.FromString(trim(vTime[1])))
					{
						if (trim(vTime[1]) == "24:00:00")
						{
							oEnd.FromString("23:59:59");
						}
						else
						{
							continue;
						}
					}

					PairInterval pair;
					pair.first = oBegin;
					pair.second = oEnd;
					vTimeInterval.push_back(pair);
				}

				if (0 != vTimeInterval.size())
				{
					m_mapNotify[sMobile] = vTimeInterval;
				}
			}
		}
	}
	return 0;
}

//启动调度线程及工作线程池
int CSmsHandler::Start()
{
	//启动调度线程
	BeginThread();
	return 0;
}

//停止工作线程池及调度线程
void CSmsHandler::Stop()
{
	//停止调度线程
	CRLog(E_APPINFO,"%s","Stop HisDataHandler Thread");
	EndThread();
}

//清理资源
void CSmsHandler::Finish()
{	
	m_deqEvt.clear();
}

void CSmsHandler::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_pCpMgr = dynamic_cast<CSmsCpMgr*>(pCpMgr);
}

int CSmsHandler::SendPacket(CPacket& pkt)
{
	try
	{
		CBroadcastPacket& oBroadcastPkt = dynamic_cast<CBroadcastPacket&>(pkt);
		m_deqCondMutex.Lock();
		m_deqEvt.push_back(oBroadcastPkt);
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

int CSmsHandler::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqEvt.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			CBroadcastPacket& pkt = m_deqEvt.front();			
			m_deqCondMutex.Unlock();

			try
			{


				HandlePacket(pkt);
			}
			catch(...)
			{
				CRLog(E_ERROR,"%s","Unknown exception!");
			}

			m_deqCondMutex.Lock();
			m_deqEvt.pop_front();
			m_deqCondMutex.Unlock();
		}
		CRLog(E_APPINFO,"%s","SmsHandler Thread exit!");
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

void CSmsHandler::HandlePacket(CBroadcastPacket& pkt)
{
	string sCmdID = pkt.GetCmdID();
	if (sCmdID == "onAlarmNotify")
	{
		CRLog(E_APPINFO,"%s", pkt.Print().c_str());

		string sSmsContent = "行情系统告警:";
		sSmsContent += CGessTime::NowToString(":");
		
		string sTmp;
		if (0 == pkt.GetParameterVal("node_id", sTmp))
		{
			sSmsContent += "|";
			sSmsContent += sTmp;
		}
		if (0 != pkt.GetParameterVal("alm_content", sTmp))
		{
			CRLog(E_APPINFO,"alm_content loss!");
			return;
		}

		
		sSmsContent += "|";
		sSmsContent += sTmp;
		if (0 == pkt.GetParameterVal("trigger_val", sTmp))
		{
			sSmsContent += "|触发值(";
			sSmsContent += sTmp;
			sSmsContent += ")";
		}

		CRLog(E_APPINFO, "Sms:%s", sSmsContent.c_str());
		SmsNotify(sSmsContent);
	}
}

void CSmsHandler::SmsNotify(const string& sSmsContent)
{
	if (m_uiSendNum > 0 && m_oSmsTimeLast.IntervalToNow() < 2*3600)
	{
		CRLog(E_APPINFO,"Last sms Time:%s !", m_oSmsTimeLast.ToString().c_str());
		return;
	}
	m_uiSendNum++;
	m_oSmsTimeLast.ToNow();


	hostent* host;
	host = gethostbyname(m_s3wName.c_str());
	if ( host == NULL)
	{
		CRLog(E_ERROR,"gethostbyname err!");
		return;
	}

	char* ip =inet_ntoa(*(struct in_addr *)*host->h_addr_list); 
	SOCKET sSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sSocket)
	{
		CRLog(E_ERROR,"sSocket err!");
		return;
	}
	sockaddr_in addrServer; 
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr(ip);
	addrServer.sin_port = htons(m_n3wPort);
	if (SOCKET_ERROR == connect( sSocket, (struct sockaddr*) &addrServer, sizeof(addrServer) ))
	{
		closesocket(sSocket);
		CRLog(E_ERROR,"connect fail!");
		return;
	}


	string sUrl = "GET /sys_port/gateway/vipsms.asp?id=";
	sUrl += m_sUserID;
	sUrl += "&pwd=";
	sUrl += m_sPwd;
	sUrl += "&to=";
	string sNotifyMobile = "";
	for (MapNotify::iterator itNotify = m_mapNotify.begin(); itNotify != m_mapNotify.end(); ++itNotify)
	{
		for (vector<PairInterval>::iterator itvPair = (*itNotify).second.begin(); itvPair != (*itNotify).second.end(); ++itvPair)
		{
			if (m_oSmsTimeLast >= (*itvPair).first && m_oSmsTimeLast <=(*itvPair).second)
			{
				if (sNotifyMobile != "")
				{
					sNotifyMobile += ",";
					sNotifyMobile += (*itNotify).first;
				}
				else
				{
					sNotifyMobile += (*itNotify).first;
				}
				break;
			}
		}
	}

	sUrl += sNotifyMobile;
	sUrl += "&content=";
	sUrl += sSmsContent;
	sUrl += "&time=\r\n\r\n";

	if (sUrl.length() != send(sSocket, sUrl.c_str(), sUrl.length(),0))
	{
		CRLog(E_APPINFO, "send err");
	}
	closesocket(sSocket);
	CRLog(E_APPINFO, "sms(%s:%d) %s" , ip, m_n3wPort, sUrl.c_str());
}

int CSmsHandler::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}