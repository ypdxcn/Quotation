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
#include "Translator.h"
#include "DataSrcCpMgr.h"
#include "MibConstant.h"
#include "BroadcastPacket.h"
#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceYC.h"
#include "ProcessInterfaceH1C.h"
#include "ProcessInterfaceH2C.h"
#include "ProcessInterfaceCmd.h"
#include "LinePacket.h"
#include "GessTimerMgrPosix.h"
#include "AbsTimerMgrWin32.h"
#include "NetMgrModule.h"
#include <sstream>
#include <iomanip>

using namespace MibConst;

//源接口+命令字 报文路由配置表
CDataSrcCpMgr::IfRouterCfg CDataSrcCpMgr::m_tblIfRouterCfg[] = 
{	
	//EnumKeyIfYC To EnumKeyDeliverMgr
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////	
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "RtnUpTickQuote"},
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "onRecvSpotQuotation"},
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "onRecvForwardQuotation"},
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "onRecvRtnInstStateUpdate"},//2013-11-29 增加合约状态转换
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "onRecvRtnDeferInstStateUpdate"},
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "onRecvRtnForwardInstStateUpdate"},
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "onRecvRtnSpotInstStateUpdate"},
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "onSysStatChange"},
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "ReqPublishQuoteLogin"},//2014-8-7 添加函数
	{0,    EnumKeyTranslator,			EnumKeyIfYC,     "quoteAnalyseTick"},//2014-8-7 添加函数

	//from H1	
	//H1 To NetMgrModule
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumNetMagModule,			EnumKeyIfH1,            "1921"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1922"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1923"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1924"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,		    "1925"}, 

	{0,    EnumNetMagModule,			EnumKeyIfH1,            "1911"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1912"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1913"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,			"1914"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,		    "1915"},
	{0,    EnumNetMagModule,			EnumKeyIfH1,		    "1916"}, 

	//from NetMgrModule	
	//NetMgrModule To H1 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		//
	{0,    EnumKeyIfH1,				EnumNetMagModule,       "1921"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1922"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1923"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1924"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1925"}, 

	{0,    EnumKeyIfH1,				EnumNetMagModule,       "1911"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1912"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1913"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1914"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1915"},
	{0,    EnumKeyIfH1,				EnumNetMagModule,		"1916"}, 

	//NetMgrModule To H2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From					CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfH2,				EnumNetMagModule,		"onEventNotify"},   //事件广播类报文
	{0,    EnumKeyIfH2,				EnumNetMagModule,		"onAlarmNotify"},   //告警广播类报文
	{0,    EnumKeyIfH2,				EnumNetMagModule,		"onNodeMibTblChg"}, //记录变化广播报文


	//from IFCMD
	//IFCMD To default 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyCmdHandler,		EnumKeyIfCmd,    			 gc_sDefaultCmdID},


};


//Telnet or Console CommandLine 对应处理函数配置表
CDataSrcCpMgr::CmdLine2Api CDataSrcCpMgr::m_CmdLine2Api[] = 
{
	//命令字			缩写			命令处理函数指针					说明
	{"replay",			"r",			&CDataSrcCpMgr::OnCmdLineReplayQuotation,	"replay the quotation by log file!\r\n\t  r 合约 间隔 个数 是否压缩 文件名, r Au(T+D) 10 1000 *"},
	{"quit",			"q",			&CDataSrcCpMgr::OnCmdLineQuit,			"quit the system"},
	{"mem",				"m",			&CDataSrcCpMgr::OnCmdLineMem,			"show mem bytes"},
	{"evt",				"e",			&CDataSrcCpMgr::OnCmdLineEvtTest,		"test evt notify"},
	{"info",			"i",			&CDataSrcCpMgr::OnCmdLineSysInfo,		"show SysInfo"},
	{"que",			   "que",			&CDataSrcCpMgr::OnCmdLineQue,			"for que"},
	{"diff",			"d",			&CDataSrcCpMgr::OnCmdLineTimeDiff,		"local pc time difference with server"},
	{"zipout",			"zo",			&CDataSrcCpMgr::OnCmdLineZipout,		"output zip pkt(on/off)"},
	{"?",				"",				&CDataSrcCpMgr::OnCmdLineHelp,			"for help"},
	{"help",			"h",			&CDataSrcCpMgr::OnCmdLineHelp,			"for help"}	
};


CDataSrcCpMgr::CDataSrcCpMgr()
:m_sProcName("DataSrc_sge")
,m_pCpInterfaceCmd(0)
,m_pCpInterfaceH1(0)
,m_pCpInterfaceH2(0)
,m_pCpInterfaceYC(0)
,m_pTranslator(0)
,m_pNetMagModule(0)
,m_pGessTimerMgr(0)
,m_uiNodeID(0)
,m_uiNodeType(0)
,m_bStop(false)
,m_uiDelayLess0s(0)
,m_uiDelayLess1s(0)
,m_uiDelayLess2s(0)
,m_uiDelayLess3s(0)
,m_uiDelayLess5s(0)
,m_uiDelayLess10s(0)
,m_uiDelayLess30s(0)
,m_uiDelayLess60s(0)
,m_uiDelayLess120s(0)
,m_uiDelayMore120s(0)
,m_uiDelayMin(0xFFFFFFFF)
,m_uiDelayMax(0)
,m_nDiff(0)
,m_uiFwdTotal(0)
{
	m_pConfig = new CConfigImpl();
}

CDataSrcCpMgr::~CDataSrcCpMgr(void)
{
	m_deqTelnets.clear();
}

//客户端协议连接点连接成功后回调
int CDataSrcCpMgr::OnConnect(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	return 0;
}

//服务端协议连接点接收到连接后回调
int CDataSrcCpMgr::OnAccept(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;
	
	return 0;
}

int CDataSrcCpMgr::OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	return 0;
}

int CDataSrcCpMgr::OnClose(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	return 0;
}

//初始化路由表
int CDataSrcCpMgr::InitRouterTbl()
{
	//配置表
	int nSize = sizeof(m_tblIfRouterCfg)/sizeof(IfRouterCfg);
	//根据路由配置表初始化内存路由表
	for ( int m = 0; m < nSize; m++ )
	{
		unsigned long ulRow = m_tblIfRouterCfg[m].ulIfFrom;
		m_tblIfRouter[ulRow].ulIfFrom = m_tblIfRouterCfg[m].ulIfFrom;
		string sCmdID = m_tblIfRouterCfg[m].sCmdID;

		switch(m_tblIfRouterCfg[m].ulIfTo)
		{
		case EnumKeyTranslator:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pTranslator));
			break;
		case EnumKeyIfYC:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceYC));
			break;
		case EnumKeyIfH1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH1));
			break;
		case EnumKeyIfH2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceH2));
			break;
		case EnumKeyIfCmd:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceCmd));
			break;
		case EnumKeyCmdHandler:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, &m_oCpCmdHandler));
			break;
		case EnumNetMagModule:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pNetMagModule));
			break;
		default:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, 0));
			break;
		}
	}

	return 0;
}

//报盘机连接点管理器初始化
int CDataSrcCpMgr::Init(const string& sProcName)
{
	m_sProcName = sProcName;

	cout << "加载配置文件..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + "DataSrc_gses";//m_sProcName;
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

	string sTmp = "";
	if (0 == m_pConfig->GetProperty("node_id",sTmp))
		m_uiNodeID = FromString<unsigned int>(sTmp);

	if (0 == m_pConfig->GetProperty("node_type",sTmp))
		m_uiNodeType = FromString<unsigned int>(sTmp);

	if (0 == m_pConfig->GetProperty("time_diff",sTmp))
		m_nDiff = FromString<int>(sTmp);
	
	//
	char szFileName[_MAX_PATH];
	::GetModuleFileName(0,szFileName, _MAX_PATH);
	sTmp = szFileName;
	sTmp = strutils::LeftSubRight(sTmp, '.');
	m_oMemShareAlive.Bind(E_PROCESS_APP);
	if (FALSE == m_oMemShareAlive.Create(sTmp.c_str()))
	{
		CRLog(E_ERROR, "m_oMemShareAlive.Create fail");
		return -1;
	}
	unsigned int uiProcessID = ::GetCurrentProcessId();
	m_oMemShareAlive.IamAlive(uiProcessID);
	m_oMemShareAlive.SetNodeID(m_uiNodeID);
	

	
	//启动定时器管理器
	CAbsTimerMgrWin32::Instance()->Init();
	m_pGessTimerMgr = CGessTimerMgrImp::Instance();
	m_pGessTimerMgr->Init(2);

	int nInterval = 4;
	string sInterval("4");
	if (0 == m_pConfig->GetProperty("hello_interval",sInterval))
	{
		nInterval = FromString<int>(sInterval);
	}
	if (nInterval > 10)
		nInterval = 10;
	if (nInterval < 2)
		nInterval = 2;

	m_oIfkTimer.Bind(this);
	m_pGessTimerMgr->CreateTimer(&m_oIfkTimer,nInterval,"KHello");


	//定时重启配置 "0,12:00:00"
	string sResetTimes = "";
	if (0 == m_pConfig->GetProperty("reset_time",sResetTimes))
	{
		bool blPara = false;
		vector<string> vWeekDayTm = explodeQuoted(",",sResetTimes);
		if (vWeekDayTm.size() == 2)
		{
			int nWeekDay = strutils::FromString<int>(vWeekDayTm[0]);
			if (nWeekDay >= 0 && nWeekDay <= 6)
			{
				CGessTime oTm;
				if (oTm.FromString(trim(vWeekDayTm[1])))
				{
					m_oResetTimer.Bind(this);
					CAbsTimerMgrWin32::Instance()->CreateWeekTimer(&m_oResetTimer,nWeekDay, oTm, "reset_timer_key");
					blPara = true;
				}
			}
		}
		else if (vWeekDayTm.size() == 1)
		{
			CGessTime oTm;
			if (oTm.FromString(trim(vWeekDayTm[0])))
			{
				m_oResetTimer.Bind(this);
				CAbsTimerMgrWin32::Instance()->CreateDayTimer(&m_oResetTimer, oTm, "reset_timer_key");
				blPara = true;
			}
		}
		
		if (!blPara)
		{
			 CRLog(E_APPINFO,"%s", "自动重启时间段配置出错");
		}
	}

	CRLog(E_NOTICE,"[%s]数据源sge...",sProcName.c_str());
	CRLog(E_NOTICE,"初始化网管代理");
	string sTblPrefix = "datasrc";
	CConfig *pCfgNetMagModule = m_pConfig->GetProperties(gc_sCfgNetMagModule);
	if (0 != pCfgNetMagModule)
	{
		if (0 == pCfgNetMagModule->GetProperty("tbl_prefix",sTmp))
			sTblPrefix = sTmp;
	}
	m_pNetMagModule = new CNetMgrModule();
	CNetMgr::Instance()->NmInit(m_pNetMagModule,sTblPrefix);
	
	
	//初始化网管代理模块
	m_pNetMagModule->Bind(this,EnumNetMagModule);
	m_pNetMagModule->Init(pCfgNetMagModule);

	//
	m_oNmoModule.Bind(this);

	CConfig *pCfgTranslator;
	pCfgTranslator = m_pConfig->GetProperties(gc_sCfgTranslator);
	if (0 != pCfgTranslator && !pCfgTranslator->IsEmpty())
	{
	}
	else
	{
		pCfgTranslator = m_pConfig;
	}
	m_pTranslator = new CTranslator();
	m_pTranslator->Bind(this,EnumKeyTranslator);
	m_pTranslator->Init(pCfgTranslator);
	CNetMgr::Instance()->Register(&m_oNmoModule,mibQueNum,mibQueNum+"."+"Translator队列");
	
	CConfig *pCfgYC;
	pCfgYC = m_pConfig->GetProperties(gc_sCfgIfYC);
	if (0 != pCfgYC && !pCfgYC->IsEmpty())
	{
		CRLog(E_NOTICE,"初始化连接点YC");
		m_pCpInterfaceYC = new CProtocolCpCli<CProcessInterfaceYC>();
		m_pCpInterfaceYC->Bind(this,EnumKeyIfYC);
		m_pCpInterfaceYC->Init(pCfgYC);
	}
	
	unsigned int uiXQueNum = 2;
	if (0 == m_pConfig->GetProperty("XQUE_NUM", sTmp))
	{
		uiXQueNum = strutils::FromString<unsigned int>(sTmp);
		if (uiXQueNum > 10)
			uiXQueNum = 2;
	}

	for (unsigned int uiIndex = 1; uiIndex <= uiXQueNum; uiIndex++)
	{
		string sCfgName = "XQUE" + strutils::ToString<unsigned int>(uiIndex);

		CConfig *pCfgWriter;
		pCfgWriter = m_pConfig->GetProperties(sCfgName);
		if (0 != pCfgWriter && !pCfgWriter->IsEmpty())
		{
		}
		else
		{
			pCfgWriter = m_pConfig;
		}
		CRLog(E_APPINFO,"初始化[%s]发布点", sCfgName.c_str());
		CXQueueIo<QUOTATION>* pWriter = new CXQueueIo<QUOTATION>();
		pWriter->Init(pCfgWriter);
		m_vecQueueIo.push_back(pWriter);

		CNetMgr::Instance()->Register(&m_oNmoModule,mibQueNum,mibQueNum+"." + sCfgName + "写缓存队列");
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueFree,gc_sMemQueFree+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueUsed,gc_sMemQueUsed+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueTotal,gc_sMemQueTotal+"." + sCfgName);
	}

	//H1接口和H2接口
	CConfig *pCfgH1;
	pCfgH1 = m_pConfig->GetProperties(gc_sCfgIfH1);
	if (0 != pCfgH1 && !pCfgH1->IsEmpty())
	{
		CRLog(E_NOTICE,"初始化连接点H1");
		m_pCpInterfaceH1 = new CProtocolCpCli<CProcessInterfaceH1C>();
		m_pCpInterfaceH1->Bind(this,EnumKeyIfH1);
		m_pCpInterfaceH1->Init(pCfgH1);
	}
	
	CConfig *pCfgH2;
	pCfgH2 = m_pConfig->GetProperties(gc_sCfgIfH2);
	if (0 != pCfgH2 && !pCfgH2->IsEmpty())
	{
		CRLog(E_NOTICE,"初始化连接点H2");
		m_pCpInterfaceH2 = new CProtocolCpCli<CProcessInterfaceH2C>();
		m_pCpInterfaceH2->Bind(this,EnumKeyIfH2);
		m_pCpInterfaceH2->Init(pCfgH2);
	}

	//初始化报文路由表
	InitRouterTbl();

	//注册需要监控项
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sSamplerInPktTotal,gc_sSamplerInPktTotal +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sSamplerFwdPktTotal,gc_sSamplerFwdPktTotal +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayMin,gc_sDelayMin +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayMax,gc_sDelayMax +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess0s,gc_sDelayLess0s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess1s,gc_sDelayLess1s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess2s,gc_sDelayLess2s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess3s,gc_sDelayLess3s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess5s,gc_sDelayLess5s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess10s,gc_sDelayLess10s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess30s,gc_sDelayLess30s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess60s,gc_sDelayLess60s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayLess120s,gc_sDelayLess120s +".0");
	CNetMgr::Instance()->Register(&m_oNmoModule,gc_sDelayMore120s,gc_sDelayMore120s +".0");
	return 0;
}

//各连接点启动
int CDataSrcCpMgr::Start()
{
	CRLog(E_NOTICE,"启动定时器管理器");
	m_pGessTimerMgr->Start();
	CRLog(E_NOTICE,"启动定时器管理器");
	CAbsTimerMgrWin32::Instance()->Start();

	//启动网络监控理模块
	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"启动网管代理模块");
		m_pNetMagModule->Start();
	}

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"启动[XQUE%d]", nCount);
			(*it)->Start();
		}
	}

	if (0 != m_pTranslator)
	{
		CRLog(E_NOTICE,"启动Translator");
		m_pTranslator->Start();
	}


	if (0 != m_pCpInterfaceYC)
	{
		CRLog(E_NOTICE,"启动连接点YC");
		m_pCpInterfaceYC->Start();
	}

	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"启动连接点H1");
		m_pCpInterfaceH1->Start();
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"启动连接点H2");
		m_pCpInterfaceH2->Start();
	}

	
	//telnet 报文处理连接点绑定
	m_oCpCmdHandler.Bind(this,EnumKeyCmdHandler);
	//网络日志回调对象绑定
	m_oNetLogHost.Bind(this);
	//网络日志处理线程绑定
	m_oNetLogThread.Bind(this);
	//命令行处理线程绑定
	m_oCmdLineThread.Bind(this);


	//
	m_oCpCmdHandler.Start();

	//telnet 命令行连接点初始化启动
	CConfig *pCfgCmd = m_pConfig->GetProperties(gc_sCfgIfCmd);
	if (0 != pCfgCmd && !pCfgCmd->IsEmpty())
	{
		m_pCpInterfaceCmd = new CProtocolCpSvr<CProcessInterfaceCmd>();
		CRLog(E_NOTICE,"初始化连接点Telnet Cmd");
		m_pCpInterfaceCmd->Init(pCfgCmd);
		m_pCpInterfaceCmd->Bind(this,EnumKeyIfCmd);

		CRLog(E_NOTICE,"启动连接点Telnet Cmd");
		m_pCpInterfaceCmd->Start();
	}


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

	//启动网络日志处理线程
	m_oNetLogThread.BeginThread();
	return 0;
}

//停止各连接点
void CDataSrcCpMgr::Stop()
{
	//停止定时器管理器
	CRLog(E_NOTICE,"停止定时器管理器");
	m_pGessTimerMgr->Stop();
	CAbsTimerMgrWin32::Instance()->Stop();

	m_oNetLogThread.EndThread();
	m_oCmdLineThread.EndThread();

	//
	m_oCpCmdHandler.Stop();
	m_oCpCmdHandler.Finish();

	if (0 != m_pCpInterfaceCmd)
	{
		CRLog(E_NOTICE,"停止连接点Telnet Cmd");
		m_pCpInterfaceCmd->Stop();

		CRLog(E_NOTICE,"清理连接点Telnet Cmd");
		m_pCpInterfaceCmd->Finish();
		delete m_pCpInterfaceCmd;
		m_pCpInterfaceCmd = 0;
	}


	
	if (0 != m_pCpInterfaceYC)
	{
		CRLog(E_NOTICE,"停止连接点YC");
		m_pCpInterfaceYC->Stop();		
	}
	
	if (0 != m_pTranslator)
	{
		CRLog(E_NOTICE,"停止Translator");
		m_pTranslator->Stop();
	}

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"停止[XQUE%d]", nCount);
			(*it)->Stop();
		}
	}

	
	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"停止连接点H1");
		m_pCpInterfaceH1->Stop();
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"停止连接点H2");
		m_pCpInterfaceH2->Stop();
	}


	//停止网络监控理模块
	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"停止网管代理模块");
		m_pNetMagModule->Stop();
	}


}

//结束清理
void CDataSrcCpMgr::Finish()
{
	m_pGessTimerMgr->Finish();
	m_pGessTimerMgr=0;
	CAbsTimerMgrWin32::Instance()->Finish();

	if (0 != m_pTranslator)
	{
		CRLog(E_NOTICE,"清理Translator");
		m_pTranslator->Finish();
		delete m_pTranslator;
		m_pTranslator = 0;
	}

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"清理[XQUE%d]", nCount);
			(*it)->Finish();
			delete (*it);
		}
	}
	m_vecQueueIo.clear();
	
	if (0 != m_pCpInterfaceYC)
	{
		CRLog(E_NOTICE,"清理连接点YC");
		m_pCpInterfaceYC->Finish();
		delete m_pCpInterfaceYC;
		m_pCpInterfaceYC = 0;
	}

	if (0 != m_pCpInterfaceH1)
	{
		CRLog(E_NOTICE,"清理连接点H1");
		m_pCpInterfaceH1->Finish();
		delete m_pCpInterfaceH1;
		m_pCpInterfaceH1 = 0;
	}

	if (0 != m_pCpInterfaceH2)
	{
		CRLog(E_NOTICE,"清理连接点H2");
		m_pCpInterfaceH2->Finish();	
		delete m_pCpInterfaceH2;
		m_pCpInterfaceH2 = 0;
	}
	

	//
	m_oCpCmdHandler.Finish();

	if (0 != m_pNetMagModule)
	{
		CRLog(E_NOTICE,"清理网管代理模块");
		m_pNetMagModule->Finish();
		delete m_pNetMagModule;
		m_pNetMagModule=0;
	}
	
	CLogger::Instance()->Finish();
	delete m_pConfig;
	m_pConfig = 0;

	CNetMgr::Instance()->NmFinish();
	CNetMgr::Instance()->UnRegisterModule(&m_oNmoModule);

	
	//
	m_oMemShareAlive.UnMap();
}

//报文转发引擎 返回值-2表示无路由
int CDataSrcCpMgr::Forward(CPacket &pkt,const unsigned long& ulKey)
{
	try
	{	
		int nRtn = -2;
		assert(EnumKeyUnknown > ulKey);
		if (EnumKeyUnknown <= ulKey)
			return -1;

		if (m_bStop)
			return 0;

		std::string sCmdID = pkt.GetCmdID();

		bool blFound = false;
		MMAP_IT it;
		RANGE_CP range = m_tblIfRouter[ulKey].mmapCmds.equal_range(sCmdID);
		for (it = range.first; it != range.second; ++it)
		{
			if (0 != (*it).second)
			{
				(*it).second->SendPacket(pkt);
				nRtn = 0;
			}
			blFound = true;
		}

		if (!blFound)
		{
			it = m_tblIfRouter[ulKey].mmapCmds.find(gc_sDefaultCmdID);
			if (it != m_tblIfRouter[ulKey].mmapCmds.end())
			{
				if (0 != (*it).second)
				{
					(*it).second->SendPacket(pkt);
					nRtn = 0;
				}
			}
		}
		return nRtn;
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"packet error!");
		return -1;
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


//进程主线程函数 此主线程退出则进程退出
int CDataSrcCpMgr::Run()
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

//CmdLine命令匹配处理
string CDataSrcCpMgr::OnCmd(const string& sCmdLine, const vector<string>& vecPara)
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
		sRtn += "DataSrc->";
		return sRtn;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		string sRtn = "\r\nDataSrc->";
		return sRtn;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		string sRtn = "\r\nDataSrc->";
		return sRtn;
	}
}

//telnet终端命令处理
int CDataSrcCpMgr::OnPacketCmd(CPacket& pkt)
{
	try
	{
		CPacketLineReq & pktLine = dynamic_cast<CPacketLineReq&>(pkt);
		string sRouteKey = pktLine.RouteKey();
		string sCmd = trim(pktLine.GetCmdID());

		vector<string> vecPara;
		vecPara.clear();
		pktLine.GetPara(vecPara);
		
		string sRsp("");
		if ("q" == sCmd || "quit" == sCmd)
		{
			sRsp += "DataSrc->";
		}
		else if (sCmd == "debug")
		{
			string sPara("");
			if (vecPara.size() > 0)
				sPara = vecPara[0];
			
			if (trim(sPara) == "on")
			{
				m_csTelnets.Lock();
				m_deqTelnets[sRouteKey] = sRouteKey;
				if (m_deqTelnets.size() == 1)
					CLogger::Instance()->RegisterNetLog(&m_oNetLogHost);
				m_csTelnets.Unlock();

				sRsp += "DataSrc->";
			}
			else if (trim(sPara) == "off")
			{
				map<string,string>::iterator itTel;	
				m_csTelnets.Lock();
				itTel = m_deqTelnets.find(sRouteKey);
				if (itTel != m_deqTelnets.end())
					m_deqTelnets.erase( itTel);

				if (m_deqTelnets.size() == 0)
					CLogger::Instance()->UnRegisterNetLog(&m_oNetLogHost);
				m_csTelnets.Unlock();

				sRsp += "DataSrc->";
			}
			else
			{
				sRsp = "Parameter err!";
				sRsp += "\r\n";
				sRsp += "DataSrc->";
			}
		}
		else
		{
			sRsp = OnCmd(sCmd, vecPara);
		}

		CPacketLineRsp pktRsp(sRsp);
		pktRsp.AddRouteKey(sRouteKey);
		return m_pCpInterfaceCmd->SendPacket(pktRsp);
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"Unknown exception!");
		return -1;
	}
}

//日志消息入线程队列
void CDataSrcCpMgr::OnNetLogMsg(const string& sMsg)
{
	m_oNetLogThread.Enque(sMsg);
}

//日志发送
int CDataSrcCpMgr::HandleNetLogMsg(const string & sNetLogMsg)
{
	string sMsg = sNetLogMsg;
	sMsg += "\r\n";
	map<string,string>::iterator itTel;	
	m_csTelnets.Lock();
	for (itTel = m_deqTelnets.begin(); itTel != m_deqTelnets.end(); ++itTel)
	{
		CPacketLineRsp pktRsp(sMsg);
		pktRsp.AddRouteKey((*itTel).first);
		if (0 != m_pCpInterfaceCmd->SendPacket(pktRsp))
		{
			string sKey = (*itTel).first;
			m_deqTelnets.erase(itTel);
			if (0 == m_deqTelnets.size())
			{
				cout << "UnRegisterNetLog(" << sKey << ")!" << endl;
				CLogger::Instance()->UnRegisterNetLog(&m_oNetLogHost);
			}

			break;
		}
	}
	m_csTelnets.Unlock();

	return 0;
}

//命令行线程处理函数 会阻塞
int CDataSrcCpMgr::HandleCmdLine(string& sIn)
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

//K接口心跳定时器回调接口
int CDataSrcCpMgr::OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan)
{
	m_oMemShareAlive.IamAlive();
	if (m_oMemShareAlive.IsIQuitCmd())
	{
		//事件通知
		string sEvtContent = "定时时间到,稍后开始重启!";

		CRLog(E_NOTICE,sEvtContent.c_str());
		NotifyEvent(sEvtContent);

		m_bStop = true;
		m_deqCondMutex.Signal();
		return -1;
	}

	static unsigned int guiTimerCount = 0;
	if (guiTimerCount % 5 == 0)
	{
		size_t nFree = 0;
		size_t nUsed = 0;
		size_t nTotal = 0;
		unsigned int uiIndex = 1;
		vector<CNMO> vNmo;
		for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
		{
			CNMO oNmo;
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))			
			{
				oNmo.m_sOid=gc_sMemQueFree;
				oNmo.m_sOidIns = gc_sMemQueFree+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex);
				oNmo.m_sValue=ToString(nFree);
				vNmo.push_back(oNmo);

				oNmo.m_sOid=gc_sMemQueUsed;
				oNmo.m_sOidIns = gc_sMemQueUsed+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex);
				oNmo.m_sValue=ToString(nUsed);
				vNmo.push_back(oNmo);


				oNmo.m_sOid=gc_sMemQueTotal;
				oNmo.m_sOidIns = gc_sMemQueTotal+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex);
				oNmo.m_sValue=ToString(nTotal);
				vNmo.push_back(oNmo);

				CNetMgr::Instance()->Report(vNmo);
			}			
			uiIndex++;
		}
	}
	guiTimerCount++;
	return 0;
}

//定时重启定时器回调接口
int CDataSrcCpMgr::OnResetTimeout(const string& sTmKey)
{
	//事件通知
	string sEvtContent = "定时时间到,稍后开始重启!";

	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

int CDataSrcCpMgr::Query(CNMO& oNmo)
{
	int nRtn = -1;
	oNmo.m_nQuality=gc_nQuolityGood;


	if(oNmo.m_sOidIns==(mibQueNum+"."+"Translator队列"))
	{
		if (0 != m_pTranslator)
		{
			oNmo.m_sValue=ToString(m_pTranslator->QueueLen());
			nRtn = 0;
		}
		return nRtn;
	}
	
	if (oNmo.m_sOid == gc_sDelayMin)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayMin);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayMax)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayMax);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess0s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess0s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess1s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess1s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess2s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess2s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess3s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess3s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess5s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess5s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess10s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess10s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess30s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess30s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess60s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess60s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayLess120s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayLess120s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayMore120s)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiDelayMore120s);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sSamplerInPktTotal)
	{
		unsigned int uiInCount = 0;
		if (0 != m_pTranslator)
		{
			uiInCount = m_pTranslator->InPktStatic();
		}
		oNmo.m_sValue=ToString<unsigned int>(uiInCount);
		nRtn = 0;
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sSamplerFwdPktTotal)
	{
		oNmo.m_sValue=ToString<unsigned int>(m_uiFwdTotal);
		nRtn = 0;
		return nRtn;
	}

	size_t nFree = 0;
	size_t nUsed = 0;
	size_t nTotal = 0;
	unsigned int uiIndex = 1;
	for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		if(oNmo.m_sOidIns==mibQueNum+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex) + "写缓存队列")
		{
			oNmo.m_sValue=ToString((*it)-> QueueLen());
			nRtn = 0;
			break;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueFree+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex))
		{
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString(nFree);
				nRtn = 0;
			}
			break;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueUsed+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex))
		{
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString(nUsed);
				nRtn = 0;
			}
			break;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueTotal+"."+ "XQUE" + strutils::ToString<unsigned int>(uiIndex))
		{
			if (0 == (*it)->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString(nTotal);
				nRtn = 0;
			}
			break;
		}
		uiIndex++;
	}
	return nRtn;
}

//帮助信息
string CDataSrcCpMgr::OnCmdLineHelp(const string& sCmdLine, const vector<string>& vecPara)
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
	sRtn += "DataSrc->";

	return sRtn;
}

string CDataSrcCpMgr::OnCmdLineReplayQuotation(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "DataSrc->\r\n";
	//CBroadcastPacket pktSrc("onRecvSpotQuotation");

	//pktSrc.AddParameter("instID","Au50g");
	//pktSrc.AddParameter("sZipBuff","mAAAAJQBKC+QAAAAjAAAAIgAAACEAAAAgAAAAHwC69p4BWzedAAAAHAAAABsAAAAaAAAAGQAAABg\r\nAAAAXAAAAFgAAABUAAAAUAAAAEwAAABIAAAARAAAAEAAAAA8AAAAOAAAADQAAAAwAAAALAAAACgA\r\nAAAkAAAAIAAAABwAAAAYAAAAFAAAABAAAAAMAAAACAAAAAQELFwAAAAA");
	//
	//m_pTranslator->SendPacket(pktSrc);
	//return sRtn;

	string sNonZip = "*";
	string sInterval = "*";
	string sInstID = "*";
	string sCount = "*";
	string sFileName = "DataSrcSge_LogProInfo";
	if (vecPara.size() > 0)
	{
		sInstID = vecPara[0];

		if (vecPara.size() > 1)
		{
			sInterval = vecPara[1];

			if (vecPara.size() > 2)
			{
				sCount = vecPara[2];				

				if (vecPara.size() > 3)
				{
					sNonZip = vecPara[3];
				
					if (vecPara.size() > 4)
					{
						sFileName = vecPara[4];
					}
				}
			}			
		}
	}

	string sDate = CGessDate::NowToString("-");
	string sFullPath = ".\\log\\";
	sFullPath += sDate;
	sFullPath += "\\";
	if (sFileName == "*")
		sFileName = "DataSrcSge_LogProInfo";
	sFullPath += sFileName;
	sFullPath += ".log";

	FILE * pFile = fopen(sFullPath.c_str(), "r");
	if (pFile == 0)
	{
		sRtn += "Open Error!\r\n";
		return sRtn;
	}

	unsigned int nPlanCount = 0xEFFFFFFF;
	if (sCount != "*")
	{
		nPlanCount = FromString<int>(sCount);
	}
	unsigned int nCount = 0;
	int c;	
	string sLine;	
	do
	{
		c = fgetc(pFile);
		if (c == '\r' || c == '\n' || c == EOF)
		{
			if (c != EOF)
			{
				bool blFlag = false;
				string::size_type nPos = sLine.find("ApiName");
				if (string::npos != nPos)
				{
					if (sLine[sLine.length()-1] != '#')
						blFlag = true;
				}
				if (blFlag)
				{
					sLine.append(1, c);
					continue;
				}
			}

			CBroadcastPacket oPkt;

			sLine = trim(sLine);
			vector<std::string> vBody;
			vBody = explodeQuoted("#", sLine);
			size_t nSize = vBody.size();
			if (2 > nSize)
			{
				sLine.clear();
				continue;
			}

			for (size_t nIndex = 0; nIndex < nSize; nIndex++)
			{
				vector<std::string> vKeyVal;
				vKeyVal = explodeQuoted("=", vBody[nIndex]);
				if (vKeyVal.size() < 2)
					continue;
				
				oPkt.AddParameter(vKeyVal[0],vKeyVal[1]);
			}

			vector<string> vKey =  oPkt.GetKeys();
			if (vKey.size() <= 2)
			{
				sLine.clear();
				continue;
			}

			string sApiName;
			if (0 == oPkt.GetParameterVal("ApiName",sApiName))
			{
				string strInstId = "";
				oPkt.GetParameterVal("instID",strInstId);
				if (sApiName == "onRecvDeferQuotation" 
					|| sApiName == "onRecvSpotQuotation" 
					|| sApiName == "onRecvForwardQuotation")
				{
					if(strInstId == sInstID || sInstID == "*")
					{
						if (sNonZip != "1")
						{
							string sZip;
							if (0 != oPkt.GetParameterVal("sZipBuff",sZip))
							{
								sLine.clear();
								continue;
							}
						}

						
						//if (m_pTranslator != 0)
						//	m_pTranslator->SendPacket(oPkt);
						Forward(oPkt, EnumKeyIfYC);
						//CRLog(E_ERROR,oPkt.Print().c_str());
						nCount++;
						if (nCount >= nPlanCount)
							break;

						int nInterval = 0;
						if (sInterval != "*")
						{
							nInterval = FromString<int>(sInterval);
							if (nInterval > 30000 || nInterval < 1)
								nInterval = 1000;
						}
						if (nInterval > 0)
							Sleep(nInterval);
					}
				}				
			}
			sLine.clear();
		}
		else
			sLine.append(1, c);
	} while (c != EOF);

	fclose(pFile);
	sRtn += "模拟:";
	sRtn += ToString<int>(nCount);
	sRtn += "条报文！";
	return sRtn;
}

//quit命令处理
string CDataSrcCpMgr::OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara)
{
	//事件通知
	string sEvtContent = "命令行发出Quit退出指令,大约3秒后退出!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);
	msleep(3);

	m_bStop = true;
	m_deqCondMutex.Signal();

	string sRtn = "DataSrc->";
	return sRtn;
}

string CDataSrcCpMgr::OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn = CSelectorIo::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CSelectorListen::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CGessTimerMgrImp::Instance()->ToString();
	sRtn += "DataSrc->";
	return sRtn;
}

//显示内存
string CDataSrcCpMgr::OnCmdLineMem(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "DataSrc->\r\n";

	bool blFalg = true;
	unsigned long ulMemAddr = 0x00;
	unsigned long ulLen = 16;
	std::stringstream ss1;
	if (vecPara.size() == 2)
	{
		ss1 << hex << vecPara[0];
		ss1 >> ulMemAddr;
		ss1 << hex << vecPara[1];
		ss1 >> ulLen;
		if (ulLen > 1024)
			ulLen = 1024;
	}
	else if (vecPara.size() == 1)
	{
		ss1 << vecPara[0];
		ss1 >> ulMemAddr;
	}
	else
	{
		blFalg = false;
	}

	
	if (blFalg)
	{
		try
		{
			std::stringstream ss2;
			const unsigned char * pMemAddr = reinterpret_cast<const unsigned char*>(ulMemAddr);
			for (unsigned long ulIndex = 0; ulIndex < ulLen; ulIndex++,pMemAddr++)
			{
				unsigned int uiVal = static_cast<unsigned int>(*pMemAddr);			
				if (ulIndex != 0 && ulIndex % 4 == 0)
				{
					ss2 << " ";
				}
				ss2 << setfill('0') << setw(2) << hex << uppercase << uiVal;
			}
			sRtn = ss2.str();
		}
		catch(...)
		{
			CRLog(E_ERROR,"读取地址超出范围");
		}
	}

	return sRtn;
}

int CDataSrcCpMgr::HandleConsolMsg(unsigned int uiMsg)
{
	
	//事件通知
	string sEvtContent = "";
	switch (uiMsg)
	{
	case CTRL_CLOSE_EVENT:
		sEvtContent = "Provider控制台窗口被强制关闭,现在退出应用!";
		break;
	case CTRL_SHUTDOWN_EVENT:
		sEvtContent = "Provider计算机关机,现在退出应用!";
		break;
	default:
		return 0;
	}
	
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

void CDataSrcCpMgr::NotifyEvent(const string& sEvt)
{
	//事件通知
	CEventSimple e;
	e.m_nEvtCategory = 0;
	e.m_nEvtType = 0;
	e.m_nGrade = 1;
	e.m_sDateTime = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");		
	e.m_sEvtContent = sEvt;
	CNetMgr::Instance()->OnEvtNotify(e);
}

string CDataSrcCpMgr::OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara)
{
	//事件通知
	string sEvtContent = "事件测试!";
	NotifyEvent(sEvtContent);

	string sRtn = "DataSrc->";
	return sRtn;
}

//时延调整
string CDataSrcCpMgr::OnCmdLineTimeDiff(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "DataSrc->";
	if (vecPara.size() != 1)
	{
		sRtn += ToString<int>(m_nDiff);
		return sRtn;
	}
	
	m_nDiff = FromString<int>(vecPara[0]);
	return sRtn;
}

//压缩报文调试输出命令
string CDataSrcCpMgr::OnCmdLineZipout(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "DataSrc->";
	if (vecPara.size() != 1)
	{
		sRtn += "para err!";
		return sRtn;
	}
	
	if (trim(vecPara[0]) == "on")
	{
		if (0 != m_pTranslator)
		{
			m_pTranslator->ZipPktOut(1);
		}
	}
	else if (trim(vecPara[0]) == "off")
	{
		if (0 != m_pTranslator)
		{
			m_pTranslator->ZipPktOut(0);
		}
	}
	else
	{
		sRtn += "para err!";
	}
	return sRtn;
}

string CDataSrcCpMgr::OnCmdLineQue(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "DataSrc->";
	
	return sRtn;
}

int CDataSrcCpMgr::ToXQueue(QUOTATION& stQuotation)
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	unsigned int uiQuoTime = stQuotation.m_uiTime/1000;
	unsigned int uiHour = uiQuoTime/10000;
	unsigned int uiMin = (uiQuoTime%10000)/100;
	unsigned int uiSec = uiQuoTime%100;
	int nDelay = st.wHour*3600 + st.wMinute * 60 + st.wSecond - uiHour*3600 - uiMin*60 - uiSec;
	//根据配置时差调整时延
	nDelay -= m_nDiff;

	//统计时延	
	if (nDelay < 0)
	{
		m_uiDelayLess0s++;
	}
	else if (nDelay <= 1)
	{
		m_uiDelayLess1s++;
	}
	else if (nDelay <= 2)
	{
		m_uiDelayLess2s++;
	}
	else if (nDelay <= 3)
	{
		m_uiDelayLess3s++;
	}
	else if (nDelay <= 5)
	{
		m_uiDelayLess5s++;
	}
	else if (nDelay <= 10)
	{
		m_uiDelayLess10s++;
	}
	else if (nDelay <= 30)
	{
		m_uiDelayLess30s++;
	}
	else if (nDelay <= 60)
	{
		m_uiDelayLess60s++;
	}
	else if (nDelay <= 120)
	{
		m_uiDelayLess120s++;
	}
	else
	{
		m_uiDelayMore120s++;
	}

	if (nDelay >= 0 && nDelay < m_uiDelayMin)
	{
		m_uiDelayMin = nDelay;
	}
	if (nDelay > 0 && nDelay > m_uiDelayMax)
	{
		m_uiDelayMax = nDelay;
	}

	//
	m_uiFwdTotal++;

	

	//分发给相应队列处理
	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>* >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_DEBUG,"Key = [%s],m_uiDelayMax =[%d]",stQuotation.m_CodeInfo.m_acCode,m_uiDelayMax);
			
			(*it)->Enque(stQuotation);
		}
	}

	return 0;
}