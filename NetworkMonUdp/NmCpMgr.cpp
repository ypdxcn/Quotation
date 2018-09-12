/******************************************************************************
版    权:深圳市雁联计算系统有限公司.
模块名称:OfferingMgr.cpp
创建者	:张伟
创建日期:2008.07.22
版    本:1.0				
模块描述:各个报盘机进程主控调度模块
主要函数:Init(...)初始化函数
         Finish() 结束清理
         Run()主控线程函数
修改记录:
******************************************************************************/

#include <iostream>
#include <fstream>
#include "Logger.h"
#include "ConfigImpl.h"
#include "strutils.h"
#include "GessTime.h"
#include "NmCpMgr.h"
#include <sstream>
#include <iomanip>

CNmCpMgr::CNmCpMgr()
:m_sProcName("NmMon")
,m_nEndThread(1)
,m_pConfig(0)
,m_nUdpSvrNum(0)
,m_uiCount(0)
{
	m_pConfig = new CConfigImpl();
}

CNmCpMgr::~CNmCpMgr(void)
{

}

//报盘机连接点管理器初始化
int CNmCpMgr::Init()
{
	cout << "加载配置文件..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + "NmMon";
	sCfgFilename = sCfgFilename + ".cfg";
	if (m_pConfig->Load(sCfgFilename) != 0)
	{
		cout << "加载配置文件[" << sCfgFilename << "]失败!" << endl;
		msleep(3);
		return -1;
	}

	cout << "初始化日志..." << endl;

	// 初始化日志
	if (CLogger::Instance()->Initial(m_pConfig->GetProperties("logger")) != 0)
	{
		cout << "Init Log failure!" << endl;
		msleep(3);
		return -1;
	}

	cout << "启动日志..." << endl;

	// 启动日志
	if (CLogger::Instance()->Start() != 0)
	{
		cout << "Log start failure!" << endl;
		msleep(3);
		return -1;
	}

	m_sIpMe = "";
	m_pConfig->GetProperty("MONC.IpMe", m_sIpMe);

	string sTmp = "";
	if (0 == m_pConfig->GetProperty("MONS.port", sTmp))
	{
		m_nUdpPortLcl = strutils::FromString<int>(sTmp);
	}

	sTmp = "";
	if (0 == m_pConfig->GetProperty("MONC.svr_list", sTmp))
	{
		vector<string> vSvr = strutils::explodeQuoted(";", sTmp);
		size_t nIdx = 0;
		size_t nSize = vSvr.size();
		size_t nMePos = nSize; 
		for (nIdx = 0; nIdx < nSize; nIdx++)
		{
			string::size_type iPos = vSvr[nIdx].find(m_sIpMe);
			if (iPos != string::npos)
			{
				nMePos = nIdx;
				break;
			}
		}

		int nCount = 0;
		for (nIdx = nMePos + 1; nIdx < nSize; nIdx++)
		{
			nCount++;
			if (nCount > 3)
			{
				break;
			}

			UDP_SVR_INFO oSvrInfo = {0};
			oSvrInfo.uiDelayMin = 0xFFFFFFFF;
			oSvrInfo.uiSeqNoReq = 1;
			vector<string> vIpPort = strutils::explodeQuoted(",", vSvr[nIdx]);
			size_t nIpPortNum = vIpPort.size();
			for (size_t nIpPortIdx = 0; nIpPortIdx < nIpPortNum; nIpPortIdx++)
			{
				vector<string> vTmp = strutils::explodeQuoted(":", vIpPort[nIpPortIdx]);
				if (vTmp.size() != 2)
				{
					continue;
				}

				UDP_IP_PORT oIp = {0};
				string sIp = strutils::trim(vTmp[0]);
				oIp.nUdpPort = strutils::FromString<int>(vTmp[1]);
				memcpy(oIp.sPeerIp, sIp.c_str(), min(sizeof(oIp.sPeerIp)-1, sIp.length()));
				if (oSvrInfo.nCount <= UDP_IP_PORT_MAX_NUM)
				{
					oSvrInfo.oSvr[oSvrInfo.nCount] = oIp;
					oSvrInfo.nCount++;
				}
			}

			if (oSvrInfo.nCount > 0)
			{
				m_vUdpSvr.push_back(oSvrInfo);
			}
		}
		
		for (nIdx = 0; nIdx < nMePos && nCount < 3; nIdx++)
		{
			nCount++;			
			if (nCount > 3)
			{
				break;
			}

			UDP_SVR_INFO oSvrInfo = {0};
			oSvrInfo.uiDelayMin = 0xFFFFFFFF;
			oSvrInfo.uiSeqNoReq = 1;
			vector<string> vIpPort = strutils::explodeQuoted(",", vSvr[nIdx]);
			size_t nIpPortNum = vIpPort.size();
			for (size_t nIpPortIdx = 0; nIpPortIdx < nIpPortNum; nIpPortIdx++)
			{
				vector<string> vTmp = strutils::explodeQuoted(":", vIpPort[nIpPortIdx]);
				if (vTmp.size() != 2)
				{
					continue;
				}

				UDP_IP_PORT oIp = {0};
				string sIp = strutils::trim(vTmp[0]);
				oIp.nUdpPort = strutils::FromString<int>(vTmp[1]);
				memcpy(oIp.sPeerIp, sIp.c_str(), min(sizeof(oIp.sPeerIp)-1, sIp.length()));
				if (oSvrInfo.nCount <= UDP_IP_PORT_MAX_NUM)
				{
					oSvrInfo.oSvr[oSvrInfo.nCount] = oIp;
					oSvrInfo.nCount++;
				}
			}

			if (oSvrInfo.nCount > 0)
			{
				m_vUdpSvr.push_back(oSvrInfo);
			}
		}
	}
	m_nUdpSvrNum = m_vUdpSvr.size();
	return 0;
}

//各连接点启动
int CNmCpMgr::Start()
{
	return 0;
}

//停止各连接点
void CNmCpMgr::Stop()
{
	m_nEndThread = 0;
}

//结束清理
void CNmCpMgr::Finish()
{
	CLogger::Instance()->Finish();
	delete m_pConfig;
	m_pConfig = 0;
}

//进程主线程函数 此主线程退出则进程退出
int CNmCpMgr::Run()
{
	m_sLcl = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addrLcl;
	memset(&addrLcl, 0x00, sizeof(addrLcl));
	addrLcl.sin_family = AF_INET;
	addrLcl.sin_addr.s_addr = INADDR_ANY;
	addrLcl.sin_port = htons(m_nUdpPortLcl);
	if (SOCKET_ERROR == bind(m_sLcl, (struct sockaddr *)&addrLcl, sizeof(addrLcl)))
	{
		CRLog(E_ERROR, "bind error!");
	}

	struct sockaddr_in addrRmt;
	memset(&addrRmt, 0x00,sizeof(addrRmt));
	socklen_t nAddrLen = sizeof(addrRmt);
	char szBuf[128] = {0};
	time_t tmLast,tmNow;
	time(&tmLast);
	struct timeval tvTimeOut = {2, 0}; // 超时	
	int nInterval = 2;
	while (1 == m_nEndThread)
	{
		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(m_sLcl, &fdRead);

		
		int nRtn = select(static_cast<int>(m_sLcl+1),&fdRead,0,0,&tvTimeOut);
		if (SOCKET_ERROR == nRtn)
		{
			CRLog(E_ERROR,"Select err:%d", GET_LAST_SOCK_ERROR());
			msleep(1);
			continue;
		}

		if (0 == nRtn)
		{
			//超时处理
			ActiveHello();
			time(&tmLast);

			tvTimeOut.tv_sec = nInterval;
			tvTimeOut.tv_usec = 0;
			continue;
		}

		time(&tmNow);
		unsigned int nDiff = static_cast<unsigned int>(difftime(tmNow,tmLast));
		if (nDiff >= nInterval)
		{
			//超时处理
			ActiveHello();
			tmLast = tmNow;

			tvTimeOut.tv_sec = nInterval;
			tvTimeOut.tv_usec = 0;
		}
		else
		{
			tvTimeOut.tv_sec = nInterval - nDiff;
			tvTimeOut.tv_usec = 0;
		}

		if (0 == FD_ISSET(m_sLcl, &fdRead))
		{//
			continue;
		}

		int nReceived = recvfrom(m_sLcl,szBuf,sizeof(szBuf),0, (struct sockaddr*)(&addrRmt), &nAddrLen);
		if (nReceived <= 0)
		{
            CRLog(E_ERROR,"recvfrom err:%d", GET_LAST_SOCK_ERROR());
			msleep(1);
			continue;
		}
		OnUdpPkt(szBuf, nReceived, addrRmt);
	}
	closesocket(m_sLcl);
	return 0;
}

void CNmCpMgr::ActiveHello()
{
	if (0 == m_nUdpSvrNum)
	{
		return;
	}
	size_t nSvrIdx = m_uiCount % m_nUdpSvrNum;

	UDP_SVR_INFO& oSvrInfo = m_vUdpSvr[nSvrIdx];
	int nIpIdx = oSvrInfo.nIdx;
	UDP_IP_PORT& oUdp = oSvrInfo.oSvr[nIpIdx];

	struct sockaddr_in addr;
	memset(&addr, 0x00,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(oUdp.nUdpPort);
	addr.sin_addr.s_addr = inet_addr(oUdp.sPeerIp);
		
	MS_PKT oPkt;
	memset(&oPkt, 0x00, sizeof(MS_PKT));
	oPkt.uiLen = sizeof(MS_PKT);
	oPkt.uiMagicNum = MS_MAGIC_NUMBER;
	oPkt.uiCmdID = MSCMD_HELLO_REQ;
	oPkt.uiKey1 = nSvrIdx;
	oPkt.nTickReq = NowTimeStamp();
	oPkt.uiSeqNo = oSvrInfo.uiSeqNoReq;

	if (oSvrInfo.uiSendNum >= 1)
	{
		if (oSvrInfo.uiSendNum < 100)
		{
			CRLog(E_ERROR, "send no rsp count:%u, %s", oSvrInfo.uiSendNum, oUdp.sPeerIp);
		}

		if (oSvrInfo.uiSendNum == 3)
		{
			oSvrInfo.uiCloseCount++;
		}
		oSvrInfo.SwitchIdx();
	}

	if (SOCKET_ERROR == sendto(m_sLcl,(char*)&oPkt,oPkt.uiLen,0,(const struct sockaddr *)&addr, sizeof(addr)))
	{
		CRLog(E_ERROR,"sendto err:%s",oUdp.sPeerIp);
		oSvrInfo.SwitchIdx();
	}
	else
	{
		oSvrInfo.uiSeqNoReq++;
	}
	oSvrInfo.uiSendNum++;

	m_uiCount++;
	if (m_uiCount % 60 == 0)
	{
		for (size_t i = 0; i < m_nUdpSvrNum; i++)
		{
			UDP_SVR_INFO& oSvr = m_vUdpSvr[i];
			CRLog(E_STATICS, "%d:close(%u),loss(%u),delay min-max-avg(%05u-%05u-%05u)ms, timediff(%04d.%03d)s.ms, %s", i, oSvr.uiCloseCount, oSvr.uiLossCount, oSvr.uiDelayMin, oSvr.uiDelayMax, oSvr.uiDelayAvg, oSvr.nTimeDiff/1000, oSvr.nTimeDiff % 1000, oSvr.oSvr[0].sPeerIp); 
			oSvr.uiDelayCount = 0;
			oSvr.uiDelayTotal = 0;
			oSvr.uiDelayMin = 0xFFFFFFFF;
			oSvr.uiDelayMax = 0;
			oSvr.uiCloseCount = 0;
			oSvr.uiSeqErrCount = 0;
		}
	}
	return;
}

void CNmCpMgr::PassiveHello(MS_PKT& oPktReq, struct sockaddr_in& addr)
{
	MS_PKT oPktRsp;
	memset(&oPktRsp, 0x00, sizeof(MS_PKT));
	oPktRsp.uiLen = sizeof(MS_PKT);
	oPktRsp.uiMagicNum = MS_MAGIC_NUMBER;
	oPktRsp.uiCmdID = MSCMD_HELLO_RSP;
	oPktRsp.uiSeqNo = oPktReq.uiSeqNo;
	oPktRsp.uiKey1 = oPktReq.uiKey1;
	oPktRsp.nTickReq = oPktReq.nTickReq;
	oPktRsp.uisTime = NowTimeStamp();

	if (SOCKET_ERROR == sendto(m_sLcl,(char*)(&oPktRsp),oPktRsp.uiLen,0,(const struct sockaddr *)&addr, sizeof(addr)))
	{
		char* pRmtIp = inet_ntoa(addr.sin_addr);
		CRLog(E_ERROR,"passive sendto err:%s", pRmtIp);
	}
	return;
}

void CNmCpMgr::OnUdpPkt(const char* pBuf, int nLen, struct sockaddr_in& addr)
{
	MS_PKT oPkt = *(PMS_PKT)(pBuf);
	if (nLen != oPkt.uiLen || nLen != sizeof(MS_PKT))
	{
		CRLog(E_ERROR, "ERROR PKT LEN:%u", oPkt.uiLen);
		return;
	}

	if (oPkt.uiCmdID >= MSCMD_CMD_UNKOWN)
	{
		CRLog(E_ERROR, "ERROR PKT CMD:%u", oPkt.uiCmdID);
		return;
	}
	
	switch (oPkt.uiCmdID)
	{
	case MSCMD_HELLO_REQ:
		PassiveHello(oPkt, addr);
		break;
	case MSCMD_HELLO_RSP:
		OnHelloRsp(oPkt);
		break;
	default:
		break;
	}
	return;
}

void CNmCpMgr::OnHelloRsp(MS_PKT& oPkt)
{
	oPkt.nTickRsp = NowTimeStamp();
	unsigned int nSvrIdx = oPkt.uiKey1;
	if (nSvrIdx >= m_nUdpSvrNum)
	{
		return;
	}

	UDP_SVR_INFO& oSvrInfo = m_vUdpSvr[nSvrIdx];
	oSvrInfo.uiSendNum = 0;

	if (oPkt.uiSeqNo <= oSvrInfo.uiSeqNoRsp)
	{
		oSvrInfo.uiSeqErrCount++;
		CRLog(E_ERROR, "SeqErr %u <= %u, %s", oPkt.uiSeqNo, oSvrInfo.uiSeqNoRsp, oSvrInfo.oSvr[oSvrInfo.nIdx].sPeerIp);
	}
	else
	{
		if (oPkt.uiSeqNo > (oSvrInfo.uiSeqNoRsp + 1))
		{
			oSvrInfo.uiLossCount += (oPkt.uiSeqNo - oSvrInfo.uiSeqNoRsp - 1);
			CRLog(E_ERROR, "Loss %u > %u, %s", oPkt.uiSeqNo, oSvrInfo.uiSeqNoRsp + 1, oSvrInfo.oSvr[oSvrInfo.nIdx].sPeerIp);
		}
		oSvrInfo.uiSeqNoRsp = oPkt.uiSeqNo;
	}

	unsigned int uiTm1 = oPkt.nTickReq/1000;
	unsigned int uimSec1 = oPkt.nTickReq % 1000;
	unsigned int uiTm2 = oPkt.nTickRsp/1000;
	unsigned int uimSec2 = oPkt.nTickRsp % 1000;
	CGessTime oTime1(uiTm1);
	CGessTime oTime2(uiTm2);
	int nDiff = oTime2 - oTime1;
	if (nDiff < 0)
	{
		nDiff = 0;
	}
	unsigned int uiDelay = 1000*nDiff + uimSec2 - uimSec1;
	
	if (oSvrInfo.uiDelayCount > 0 && oSvrInfo.uiDelayCount % 60 == 0)
	{
		int nmSecondLcl = NowMSec();
		int nmSecondRmt = MSecFromTimestamp(oPkt.uisTime);
		oSvrInfo.nTimeDiff = nmSecondRmt + oSvrInfo.uiDelayAvg - nmSecondLcl;
	}
	
	oSvrInfo.uiDelayTotal += uiDelay;
	oSvrInfo.uiDelayCount++;
	oSvrInfo.uiDelayAvg = oSvrInfo.uiDelayTotal/oSvrInfo.uiDelayCount;
	
	if (oSvrInfo.uiDelayMax < uiDelay)
	{
		oSvrInfo.uiDelayMax = uiDelay;
	}

	if (oSvrInfo.uiDelayMin > uiDelay)
	{
		oSvrInfo.uiDelayMin = uiDelay;
	}
}


int CNmCpMgr::NowTimeStamp()
{
	int nTimeStamp = 0;
#ifdef WIN32
	SYSTEMTIME st;
	::GetLocalTime(&st);
	nTimeStamp = st.wHour*10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;
#else
	struct timeval tv;
    gettimeofday(&tv,0);

	time_t rawtime = tv.tv_sec;
	struct tm * timeinfo = localtime (&rawtime);
	nTimeStamp = timeinfo->tm_hour*10000000 + timeinfo->tm_min * 100000 + timeinfo->tm_sec * 1000 + tv.tv_usec/1000;
#endif
	return nTimeStamp;
}

int CNmCpMgr::MSecFromTimestamp(int nTimeStamp)
{
	int nHour = nTimeStamp / 10000000;
	int nMinute = (nTimeStamp % 10000000) / 100000;
	int nSecond = (nTimeStamp % 100000) / 1000;
	int nMillSecond = nTimeStamp % 1000;
	int nimSecond = nHour*3600*1000 + nMinute * 60 * 1000 + nSecond * 1000 + nMillSecond;
	return nimSecond;
}


int CNmCpMgr::NowMSec()
{
	int nTimeStamp = 0;
#ifdef WIN32
	SYSTEMTIME st;
	::GetLocalTime(&st);
	nTimeStamp = st.wHour*3600*1000 + st.wMinute*60*1000 + st.wSecond*1000 + st.wMilliseconds;
#else
	struct timeval tv;
    gettimeofday(&tv,0);

	time_t rawtime = tv.tv_sec;
	struct tm * timeinfo = localtime (&rawtime);
	nTimeStamp = timeinfo->tm_hour*3600*1000 + timeinfo->tm_min*60*1000 + timeinfo->tm_sec*1000 + tv.tv_usec/1000;
#endif
	return nTimeStamp;
}