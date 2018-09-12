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
#include "NmService.h"
#include "NmCpMgr.h"
#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceMonc.h"
#include "ProcessInterfaceMons.h"
#include <sstream>
#include <iomanip>


//Telnet or Console CommandLine 对应处理函数配置表
CNmCpMgr::CmdLine2Api CNmCpMgr::m_CmdLine2Api[] = 
{
	//命令字			缩写			命令处理函数指针					说明
	{"quit",			"q",			&CNmCpMgr::OnCmdLineQuit,			"quit the system"},
	{"?",				"",				&CNmCpMgr::OnCmdLineHelp,			"for help"},
	{"help",			"h",			&CNmCpMgr::OnCmdLineHelp,			"for help"}	
};


CNmCpMgr::CNmCpMgr()
:m_sProcName("NmMon")
,m_pCpInterfaceMons(0)
,m_pCfgMonc1(0)
,m_pCpInterfaceMonc1(0)
,m_pCfgMonc2(0)
,m_pCpInterfaceMonc2(0)
,m_pCfgMonc3(0)
,m_pCpInterfaceMonc3(0)
//,m_pGessTimerMgr(0)
,m_pNmService(0)
,m_bStop(false)
{
	m_pConfig = new CConfigImpl();
}

CNmCpMgr::~CNmCpMgr(void)
{

}

//客户端协议连接点连接成功后回调
int CNmCpMgr::OnConnect(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	if (0 == nFlag)
	{
		m_pNmService->HandleConnect(ulKey, sPeerIp);
	}
	return 0;
}

//服务端协议连接点接收到连接后回调
int CNmCpMgr::OnAccept(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;
	
	return 0;
}

int CNmCpMgr::OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	return 0;
}

int CNmCpMgr::OnClose(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	m_pNmService->HandleClose(ulKey);
	return 0;
}

//报盘机连接点管理器初始化
int CNmCpMgr::Init(const string& sProcName)
{
	m_sProcName = sProcName;

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
		cout << "Init Log [" << m_sProcName << "] failure!" << endl;
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
		
	//
	//char szFileName[_MAX_PATH];
	//::GetModuleFileName(0,szFileName, _MAX_PATH);
	//string sTmp = szFileName;
	//sTmp = strutils::LeftSubRight(sTmp, '.');
	//m_oMemShareAlive.Bind(E_PROCESS_APP);
	//if (FALSE == m_oMemShareAlive.Create(sTmp.c_str()))
	//{
	//	CRLog(E_ERROR, "m_oMemShareAlive.Create fail");
	//	return -1;
	//}
	//unsigned int uiProcessID = ::GetCurrentProcessId();
	//m_oMemShareAlive.IamAlive(uiProcessID);
	//m_oMemShareAlive.SetNodeID(0xabcdef);
	//

	//
	////启动定时器管理器
	//m_pGessTimerMgr = CGessTimerMgrImp::Instance();
	//m_pGessTimerMgr->Init(2);

	//int nInterval = 4;
	//string sInterval("4");
	//if (0 == m_pConfig->GetProperty("hello_interval",sInterval))
	//{
	//	nInterval = FromString<int>(sInterval);
	//}
	//if (nInterval > 10)
	//	nInterval = 10;
	//if (nInterval < 2)
	//	nInterval = 2;

	//m_oIfkTimer.Bind(this);
	//m_pGessTimerMgr->CreateTimer(&m_oIfkTimer,nInterval,"KHello");



	CConfig *pCfgNmService;
	pCfgNmService = m_pConfig->GetProperties(gc_sCfgNmService);
	if (0 != pCfgNmService && !pCfgNmService->IsEmpty())
	{
	}
	else
	{
		pCfgNmService = m_pConfig;
	}
	m_pNmService = new CNmService(this);
	m_pNmService->Init(pCfgNmService);

	CConfig *pCfgMons;
	pCfgMons = m_pConfig->GetProperties(gc_sCfgIfMons);
	if (0 != pCfgMons && !pCfgMons->IsEmpty())
	{
		CRLog(E_NOTICE,"初始化连接点Mons");
		m_pCpInterfaceMons = new CProtocolCpSvr<CProcessInterfaceMons>();
		m_pCpInterfaceMons->Bind(this,EnumKeyIfMons);
		m_pCpInterfaceMons->Init(pCfgMons);
	}

	string sIpMe = "";
	m_pConfig->GetProperty("MONC.IpMe", sIpMe);

	string sTmp = "";
	vector<string> vSvr;
	if (0 == m_pConfig->GetProperty("MONC.svr_list", sTmp))
	{
		vSvr = strutils::explodeQuoted(";", sTmp);
		size_t nIdx = 0;
		size_t nSize = vSvr.size();
		size_t nMePos = nSize; 
		for (nIdx = 0; nIdx < nSize; nIdx++)
		{
			string::size_type iPos = string::npos; 
			iPos = vSvr[nIdx].find(sIpMe);
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
			if (1 == nCount)
			{
				m_pCfgMonc1 = new CConfigImpl;
				m_pCfgMonc1->SetProperty("ip_port", vSvr[nIdx]);

				CRLog(E_NOTICE,"初始化连接点Monc:%s", vSvr[nIdx].c_str());
				m_pCpInterfaceMonc1 = new CProtocolCpCli<CProcessInterfaceMonc>();
				m_pCpInterfaceMonc1->Bind(this, EnumKeyIfMonc1);
				m_pCpInterfaceMonc1->Init(m_pCfgMonc1);
			}
			else if (2 == nCount)
			{
				m_pCfgMonc2 = new CConfigImpl;
				m_pCfgMonc2->SetProperty("ip_port", vSvr[nIdx]);

				CRLog(E_NOTICE,"初始化连接点Monc:%s", vSvr[nIdx].c_str());
				m_pCpInterfaceMonc2 = new CProtocolCpCli<CProcessInterfaceMonc>();
				m_pCpInterfaceMonc2->Bind(this, EnumKeyIfMonc2);
				m_pCpInterfaceMonc2->Init(m_pCfgMonc2);
			}
			else if (3 == nCount)
			{
				m_pCfgMonc3 = new CConfigImpl;
				m_pCfgMonc3->SetProperty("ip_port", vSvr[nIdx]);

				CRLog(E_NOTICE,"初始化连接点Monc:%s", vSvr[nIdx].c_str());
				m_pCpInterfaceMonc3 = new CProtocolCpCli<CProcessInterfaceMonc>();
				m_pCpInterfaceMonc3->Bind(this, EnumKeyIfMonc3);
				m_pCpInterfaceMonc3->Init(m_pCfgMonc3);
			}
			else
			{
				break;
			}
		}
		
		for (nIdx = 0; nIdx < nMePos && nCount < 3; nIdx++)
		{
			nCount++;
			if (1 == nCount)
			{
				m_pCfgMonc1 = new CConfigImpl;
				m_pCfgMonc1->SetProperty("ip_port", vSvr[nIdx]);

				CRLog(E_NOTICE,"初始化连接点Monc:%s", vSvr[nIdx].c_str());
				m_pCpInterfaceMonc1 = new CProtocolCpCli<CProcessInterfaceMonc>();
				m_pCpInterfaceMonc1->Bind(this, EnumKeyIfMonc1);
				m_pCpInterfaceMonc1->Init(m_pCfgMonc1);
			}
			else if (2 == nCount)
			{
				m_pCfgMonc2 = new CConfigImpl;
				m_pCfgMonc2->SetProperty("ip_port", vSvr[nIdx]);

				CRLog(E_NOTICE,"初始化连接点Monc:%s", vSvr[nIdx].c_str());
				m_pCpInterfaceMonc2 = new CProtocolCpCli<CProcessInterfaceMonc>();
				m_pCpInterfaceMonc2->Bind(this, EnumKeyIfMonc2);
				m_pCpInterfaceMonc2->Init(m_pCfgMonc2);
			}
			else if (3 == nCount)
			{
				m_pCfgMonc3 = new CConfigImpl;
				m_pCfgMonc3->SetProperty("ip_port", vSvr[nIdx]);

				CRLog(E_NOTICE,"初始化连接点Monc:%s", vSvr[nIdx].c_str());
				m_pCpInterfaceMonc3 = new CProtocolCpCli<CProcessInterfaceMonc>();
				m_pCpInterfaceMonc3->Bind(this, EnumKeyIfMonc3);
				m_pCpInterfaceMonc3->Init(m_pCfgMonc3);
			}
			else
			{
				break;
			}
		}
	}
	return 0;
}

//各连接点启动
int CNmCpMgr::Start()
{
	//CRLog(E_NOTICE,"启动定时器管理器");
	//m_pGessTimerMgr->Start();


	if (0 != m_pNmService)
	{
		CRLog(E_NOTICE,"启动mNmService");
		m_pNmService->Start();
	}


	if (0 != m_pCpInterfaceMons)
	{
		CRLog(E_NOTICE,"启动连接点Mons");
		m_pCpInterfaceMons->Start();
	}

	if (0 != m_pCpInterfaceMonc1)
	{
		CRLog(E_NOTICE,"启动连接点Monc1");
		m_pCpInterfaceMonc1->Start();
	}

	if (0 != m_pCpInterfaceMonc2)
	{
		CRLog(E_NOTICE,"启动连接点Monc2");
		m_pCpInterfaceMonc2->Start();
	}

	if (0 != m_pCpInterfaceMonc3)
	{
		CRLog(E_NOTICE,"启动连接点Monc3");
		m_pCpInterfaceMonc3->Start();
	}

	
	//命令行处理线程绑定
	m_oCmdLineThread.Bind(this);

	string sCmdFlag("0");
	int nCmdFlag = 0;
	if (0 == m_pConfig->GetProperty("cmdline",sCmdFlag))
	{
		nCmdFlag = FromString<int>(sCmdFlag);
	}
	if (1 == nCmdFlag)
	{
		//启动命令行处理线程
		m_oCmdLineThread.BeginThread();
	}
	return 0;
}

//停止各连接点
void CNmCpMgr::Stop()
{	
	//停止定时器管理器
	//CRLog(E_NOTICE,"停止定时器管理器");
	//m_pGessTimerMgr->Stop();
		
	m_oCmdLineThread.EndThread();

	if (0 != m_pCpInterfaceMons)
	{
		CRLog(E_NOTICE,"停止连接点Mons");
		m_pCpInterfaceMons->Stop();		
	}

	if (0 != m_pCpInterfaceMonc1)
	{
		CRLog(E_NOTICE,"停止连接点Monc1");
		m_pCpInterfaceMonc1->Stop();
	}

	if (0 != m_pCpInterfaceMonc2)
	{
		CRLog(E_NOTICE,"停止连接点Monc2");
		m_pCpInterfaceMonc2->Stop();
	}

	if (0 != m_pCpInterfaceMonc3)
	{
		CRLog(E_NOTICE,"停止连接点Monc3");
		m_pCpInterfaceMonc3->Stop();
	}

	if (0 != m_pNmService)
	{
		CRLog(E_NOTICE,"停止NmService");
		m_pNmService->Stop();
	}
}

//结束清理
void CNmCpMgr::Finish()
{
	//m_pGessTimerMgr->Finish();
	//m_pGessTimerMgr=0;

	if (0 != m_pNmService)
	{
		CRLog(E_NOTICE,"清理NmService");
		m_pNmService->Finish();
		delete m_pNmService;
		m_pNmService = 0;
	}
	
	if (0 != m_pCpInterfaceMons)
	{
		CRLog(E_NOTICE,"清理连接点Mons");
		m_pCpInterfaceMons->Finish();
		delete m_pCpInterfaceMons;
		m_pCpInterfaceMons = 0;
	}

	if (0 != m_pCpInterfaceMonc1)
	{
		CRLog(E_NOTICE,"清理连接点Monc1");
		m_pCpInterfaceMonc1->Finish();
		delete m_pCpInterfaceMonc1;
		m_pCpInterfaceMonc1 = 0;
		delete m_pCfgMonc1;
		m_pCfgMonc1 = 0;
	}

	if (0 != m_pCpInterfaceMonc2)
	{
		CRLog(E_NOTICE,"清理连接点Monc2");
		m_pCpInterfaceMonc2->Finish();
		delete m_pCpInterfaceMonc2;
		m_pCpInterfaceMonc2 = 0;
		delete m_pCfgMonc2;
		m_pCfgMonc2 = 0;
	}

	if (0 != m_pCpInterfaceMonc3)
	{
		CRLog(E_NOTICE,"清理连接点Monc3");
		m_pCpInterfaceMonc3->Finish();
		delete m_pCpInterfaceMonc3;
		m_pCpInterfaceMonc3 = 0;
		delete m_pCfgMonc3;
		m_pCfgMonc3 = 0;
	}

	CLogger::Instance()->Finish();
	delete m_pConfig;
	m_pConfig = 0;
	
	//
	//m_oMemShareAlive.UnMap();
}

//报文转发引擎 返回值-2表示无路由
int CNmCpMgr::Forward(CPacket &pkt,const unsigned long& ulKey)
{
	try
	{
		int nRtn = -2;
		assert(EnumKeyUnknown > ulKey);
		if (EnumKeyUnknown <= ulKey)
			return -1;

		if (m_bStop)
			return 0;

		CBinBlockPkt& oPkt = dynamic_cast<CBinBlockPkt&>(pkt);				
		if (EnumKeyIfMons == ulKey)
		{
			m_pNmService->HandleMonsPkt(oPkt);
		}
		else
		{
			m_pNmService->HandleMoncPkt(oPkt, ulKey);
		}
		return nRtn;
	}
	catch (std::bad_cast& bc)
	{
		CRLog(E_ERROR,"packet error!");
		return -1;
	}
	catch (std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}


//进程主线程函数 此主线程退出则进程退出
int CNmCpMgr::Run()
{
	try
	{
		while ( !m_bStop )
		{
			m_deqCondMutex.Lock();
			while(!m_bStop)
				m_deqCondMutex.Wait();
			
			if (m_bStop)
			{
				m_deqCondMutex.Unlock();
				break;
			}

		}

		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}

int CNmCpMgr::ToMoncx(CPacket &oPacket,const unsigned long& ulKey)
{
	if (EnumKeyIfMonc1 == ulKey)
	{
		if (0 != m_pCpInterfaceMonc1)
		{
			m_pCpInterfaceMonc1->SendPacket(oPacket);
		}
	}
	else if (EnumKeyIfMonc2 == ulKey)
	{
		if (0 != m_pCpInterfaceMonc2)
		{
			m_pCpInterfaceMonc2->SendPacket(oPacket);
		}
	}
	else if (EnumKeyIfMonc3 == ulKey)
	{
		if (0 != m_pCpInterfaceMonc3)
		{
			m_pCpInterfaceMonc3->SendPacket(oPacket);
		}
	}
	return 0;
}

int CNmCpMgr::ToMons(CPacket &oPacket)
{
	if (0 != m_pCpInterfaceMons)
	{
		m_pCpInterfaceMons->SendPacket(oPacket);
	}
	return 0;
}


//K接口心跳定时器回调接口
//int CNmCpMgr::OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan)
//{
//	m_oMemShareAlive.IamAlive();
//	if (m_oMemShareAlive.IsIQuitCmd())
//	{
//		m_bStop = true;
//		m_deqCondMutex.Signal();
//		return -1;
//	}
//	return 0;
//}



//CmdLine命令匹配处理
string CNmCpMgr::OnCmd(const string& sCmdLine, const vector<string>& vecPara)
{
	try
	{
		std::string sCmdID = trim(sCmdLine);
	
		int nSize = sizeof(m_CmdLine2Api)/sizeof(CmdLine2Api);
		for ( int i = 0 ; i < nSize ; i++ )
		{
			if ( m_CmdLine2Api[i].sCmdName == sCmdID || (sCmdID != "" && m_CmdLine2Api[i].sCmdAbbr == sCmdID) )
			{
				if (m_CmdLine2Api[i].pMemberFunc == 0)
					break;

				return (this->*(m_CmdLine2Api[i].pMemberFunc))(sCmdLine, vecPara);				
			}
		}
		
		string sRtn("");
		if (sCmdID != "")
		{
			sRtn += "err command!\r\n";
		}
		sRtn += "NmMon->";
		return sRtn;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		string sRtn = "\r\nNmMon->";
		return sRtn;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		string sRtn = "\r\nNmMon->";
		return sRtn;
	}
}


//命令行线程处理函数 会阻塞
int CNmCpMgr::HandleCmdLine(string& sIn)
{
	try
	{
		char cIn = 0;
		cin.get(cIn);
		if (cIn == '\n')
		{				
			string sCmd("");

			sIn = trim(sIn);
			vector<string> vPara;
			vPara = explodeQuoted(" ", sIn);
			if (vPara.size() > 1)
			{
				sCmd = vPara[0];
				vPara.erase(vPara.begin());
			}
			else
			{
				sCmd = sIn;
				vPara.clear();
			}

			string sOut = OnCmd(sCmd, vPara);
			cout << sOut.c_str();
			sIn.clear();
		}
		else
		{
			sIn.append(1,cIn);
		}

		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}


//帮助信息
string CNmCpMgr::OnCmdLineHelp(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn("");
	int nSize = sizeof(m_CmdLine2Api)/sizeof(CmdLine2Api);
	for ( int i = 0 ; i < nSize ; i++ )
	{
		sRtn += m_CmdLine2Api[i].sCmdName;
		if (m_CmdLine2Api[i].sCmdAbbr != "")
		{
			sRtn += "/";
			sRtn += m_CmdLine2Api[i].sCmdAbbr;
		}
		sRtn += ":";
		sRtn += m_CmdLine2Api[i].sHelp;
		sRtn += "\r\n";
	}
	sRtn += "NmMon->";

	return sRtn;
}

//quit命令处理
string CNmCpMgr::OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara)
{
	//事件通知
	string sEvtContent = "命令行发出Quit退出指令,大约3秒后退出!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	msleep(3);

	m_bStop = true;
	m_deqCondMutex.Signal();

	string sRtn = "NmMon->";
	return sRtn;
}
