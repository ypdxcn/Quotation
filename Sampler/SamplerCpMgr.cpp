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
#include "Logger.h"
#include "ConfigImpl.h"
#include "SamplerCpMgr.h"
#include "MibConstant.h"
#include "ServiceHandler.h"
#include "Translator.h"
#include "IfSvAgent.h"
#include "ProtocolConnectPoint.h"
#include "ProcessInterfaceZC.h"
#include "ProcessInterfaceZS.h"
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
CSamplerCpMgr::IfRouterCfg CSamplerCpMgr::m_tblIfRouterCfg[] = 
{	
	//from EnumKeyIfZC1
	//EnumKeyIfZC1 To EnumKeyTranslator 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyTranslator,			EnumKeyIfZC1,     "00000006"},
	//EnumKeyIfZC1 To EnumKeyServiceHandler1 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler1,		EnumKeyIfZC1,     gc_sDefaultCmdID},

	
	//作为银行端,上级节点的监控查询请求 只可能从上级cvg收到此SV_REQ报文
	{0,		   EnumKeySvAgent,			EnumKeyIfZC1,     "00000011"},
	//作为银行端,上级节点的监控查询应答 应答向上转发
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "80000011"},
	//作为银行端,监控告警事件 事件向上转发
	{0,		   EnumKeyIfZC1,			EnumKeySvAgent,   "00000012"},


	//from EnumKeyServiceHandler1
	//EnumKeyServiceHandler1 To EnumKeyIfZC1 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC1,				EnumKeyServiceHandler1,     gc_sDefaultCmdID},

	//from EnumKeyIfZC2
	//EnumKeyIfZC1 To EnumKeyTranslator 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyTranslator,			EnumKeyIfZC2,     "00000006"},
	//EnumKeyIfZC1 To EnumKeyServiceHandler1 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyServiceHandler2,		EnumKeyIfZC2,     gc_sDefaultCmdID},

	//from EnumKeyServiceHandler1
	//EnumKeyServiceHandler1 To EnumKeyIfZC1 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyIfZC2,				EnumKeyServiceHandler2,     gc_sDefaultCmdID},

	//from H1	
	//H1 To NetMgrModule
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeySvAgent,			EnumKeyIfH1,            "1921"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1922"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1923"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,			"1924"},
	{0,    EnumKeySvAgent,			EnumKeyIfH1,		    "1925"}, 


	//from NetMgrModule	
	//NetMgrModule To H1 
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From                    CmdID		//
	{0,    EnumKeySvAgent,				EnumNetMagModule,       "1921"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1922"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1923"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1924"},
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"1925"}, 


	//NetMgrModule To H2
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From					CmdID		///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onEventNotify"},   //事件广播类报文
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onAlarmNotify"},   //告警广播类报文
	{0,    EnumKeySvAgent,				EnumNetMagModule,		"onNodeMibTblChg"}, //记录变化广播报文

	//from IFCMD
	//IFCMD To default 缺省路由
	///////////////////////////////////////////////////////////////////////////
	//Obj      To						From             CmdID			  	///
	///////////////////////////////////////////////////////////////////////////
	{0,    EnumKeyCmdHandler,		EnumKeyIfCmd,    			 gc_sDefaultCmdID},

    // added by Jerry Lee, 2010-12-21, 收到历史数据命令后的处理流程
    {0,	   EnumKeyTranslator,		EnumKeyIfZC1,     "00000007"},

    // added by Jerry Lee, 2011-1-18, 收到tick数据命令后的处理流程
    {0,	   EnumKeyTranslator,		EnumKeyIfZC1,     "00000008"},

    // added by Jerry Lee, 2011-2-24, 收到info数据命令后的处理流程
    {0,	   EnumKeyTranslator,		EnumKeyIfZC1,     "00000009"}
};

//Telnet or Console CommandLine 对应处理函数配置表
CSamplerCpMgr::CmdLine2Api CSamplerCpMgr::m_CmdLine2Api[] = 
{
	//命令字			缩写			命令处理函数指针					说明
	{"quit",			"q",			&CSamplerCpMgr::OnCmdLineQuit,			"quit the system"},
	{"mem",				"m",			&CSamplerCpMgr::OnCmdLineMem,			"show mem bytes"},
	{"evt",				"e",			&CSamplerCpMgr::OnCmdLineEvtTest,		"test evt notify"},
	{"info",			"i",			&CSamplerCpMgr::OnCmdLineSysInfo,		"show SysInfo"},
	{"que",			   "que",			&CSamplerCpMgr::OnCmdLineQue,			"for que"},
	{"?",				"",				&CSamplerCpMgr::OnCmdLineHelp,			"for help"},
	{"help",			"h",			&CSamplerCpMgr::OnCmdLineHelp,			"for help"}	
};


// added by Jerry Lee, 2010-12-24
// 历史数据结构定义, 由sampler读取历史数据时使用
typedef struct tagFixedHistoryData
{
    HistoryDataHeader header; 
    char content[0];        // 数据 
} FixedHistoryData;
//

// added by Jerry Lee, 2011-1-18
// tick数据结构定义, 由sampler读取tick数据时使用
typedef struct tagFixedTickData
{
    HistoryDataHeader header;   // tick数据包头
    int fileSize;               // 原始文件的大小
    int seqNo;                  // 数据包序列号, 从0开始计数
    int zipSize;                // 压缩后文件的大小
    char content[0];            // 数据 
} FixedTickData;
//


CSamplerCpMgr::CSamplerCpMgr()
:m_sProcName("Sampler")
,m_pCpInterfaceCmd(0)
,m_pCpInterfaceH1(0)
,m_pCpInterfaceH2(0)
,m_pCpInterfaceZS(0)
,m_pCpInterfaceZC1(0)
,m_pCpInterfaceZC2(0)
,m_pTranslator(0)
,m_pServiceHandler1(0)
,m_pServiceHandler2(0)
,m_pNetMagModule(0)
,m_pSvAgent(0)
//,m_pQueueIo(0)
,m_uiNodeID(0)
,m_uiNodeType(0)
,m_bStop(false)
,m_uiFwdTotal(0)
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
,m_bTimeDelayFilter(true)
{
	m_pConfig = new CConfigImpl();
}

CSamplerCpMgr::~CSamplerCpMgr(void)
{
	m_deqTelnets.clear();
}

//客户端协议连接点连接成功后回调
int CSamplerCpMgr::OnConnect(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	return 0;
}

//服务端协议连接点接收到连接后回调
int CSamplerCpMgr::OnAccept(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;
	
	return 0;
}

int CSamplerCpMgr::OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag)
{
	return 0;
}

int CSamplerCpMgr::OnClose(const unsigned long& ulKey, const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort)
{
	if (ulKey >= EnumKeyUnknown)
		return -1;

	return 0;
}

//初始化路由表
int CSamplerCpMgr::InitRouterTbl()
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
		case EnumKeyServiceHandler1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler1));
			break;
		case EnumKeyServiceHandler2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pServiceHandler2));
			break;
		case EnumKeyIfZC1:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC1));
			break;
		case EnumKeyIfZC2:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pCpInterfaceZC2));
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
		case EnumKeySvAgent:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, m_pSvAgent));
			break;
		default:
			m_tblIfRouter[ulRow].mmapCmds.insert(MMAP_CP::value_type(m_tblIfRouterCfg[m].sCmdID, 0));
			break;
		}
	}

	return 0;
}

//报盘机连接点管理器初始化
int CSamplerCpMgr::Init(const string& sProcName)
{
	m_sProcName = sProcName;

	cout << "加载配置文件..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + "Sampler";//m_sProcName;
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

	if (0 == m_pConfig->GetProperty("FilterDelayTime",sTmp))
	{
		if(sTmp == "0")
			m_bTimeDelayFilter = false;
	}


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
	
	//
	CGessTimerMgrImp::Instance()->Init(2);
	CAbsTimerMgrWin32::Instance()->Init();

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
	CGessTimerMgrImp::Instance()->CreateTimer(&m_oIfkTimer,nInterval,"KHello");

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
		
		if (!blPara)
		{
			 CRLog(E_APPINFO,"%s", "自动重启时间段配置出错");
		}
	}
	
	CRLog(E_NOTICE,"[%s]行情采集器...",sProcName.c_str());
	CRLog(E_NOTICE,"初始化网管代理");
	string sTblPrefix = "sampler";
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


	/*CConfig *pCfgXQueue;
	pCfgXQueue = m_pConfig->GetProperties(gc_sCfgXQueue);
	if (0 != pCfgXQueue && !pCfgXQueue->IsEmpty())
	{
		CRLog(E_NOTICE,"初始化连接点XQueue");
		m_pQueueIo = new CXQueueIo<QUOTATION>();
		m_pQueueIo->Init(pCfgXQueue);
	}

	if (0 != m_pQueueIo)
	{
		string sCfgName = "XQUE.QUOTATION";
		CNetMgr::Instance()->Register(&m_oNmoModule,mibQueNum,mibQueNum+"." + sCfgName + ".写缓存队列");
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueFree,gc_sMemQueFree+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueUsed,gc_sMemQueUsed+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueTotal,gc_sMemQueTotal+"." + sCfgName);
	}*/

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



    // add by Jerry Lee, 2010-12-24, 保存历史数据的内存队列
    CConfig *pCfgXQueueHis;
    pCfgXQueueHis = m_pConfig->GetProperties(gc_sCfgXQueueHis);
    if (0 != pCfgXQueueHis && !pCfgXQueueHis->IsEmpty())
    {
        CRLog(E_NOTICE,"初始化历史数据连接点XQueue");
        m_pQueHisData = new CXQueueIo<HistoryDataBuf>();
        //m_pQueHisData->Init(pCfgXQueue); // mod by Jerry Lee, 2010-12-31, 配置指针传递错误
        m_pQueHisData->Init(pCfgXQueueHis); 
    }

    //	
    if (0 != m_pQueHisData)
	{
		string sCfgName = "XQUE.HISDATA";
		CNetMgr::Instance()->Register(&m_oNmoModule,mibQueNum,mibQueNum+"." + sCfgName + ".写缓存队列");
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueFree,gc_sMemQueFree+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueUsed,gc_sMemQueUsed+"." + sCfgName);
		CNetMgr::Instance()->Register(&m_oNmoModule,gc_sMemQueTotal,gc_sMemQueTotal+"." + sCfgName);
	}

	CConfig *pCfgTranslator;
	pCfgTranslator = m_pConfig->GetProperties(gc_sCfgTranslator);
	if (0 != pCfgTranslator && !pCfgTranslator->IsEmpty())
	{
	}
	else
	{
		pCfgTranslator = m_pConfig;
	}
	CRLog(E_NOTICE,"初始化连接点Translator");
	m_pTranslator = new CTranslator();
	m_pTranslator->Bind(this,EnumKeyTranslator);
	m_pTranslator->Init(pCfgTranslator);
	CNetMgr::Instance()->Register(&m_oNmoModule,mibQueNum,mibQueNum+"."+"Translator队列");


	CConfig *pCfgServiceHandler;
	pCfgServiceHandler = m_pConfig->GetProperties(gc_sCfgService);
	if (0 != pCfgServiceHandler && !pCfgServiceHandler->IsEmpty())
	{
	}
	else
	{
		pCfgServiceHandler = m_pConfig;
	}

	CConfig *pCfgZC1;
	pCfgZC1 = m_pConfig->GetProperties(gc_sCfgIfZC1);
	if (0 != pCfgZC1 && !pCfgZC1->IsEmpty())
	{
		CRLog(E_NOTICE,"初始化连接点ZC1");
		m_pCpInterfaceZC1 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC1->Bind(this,EnumKeyIfZC1);
		m_pCpInterfaceZC1->Init(pCfgZC1);

		CRLog(E_NOTICE,"初始化连接点ServiceHandler1");
		m_pServiceHandler1 = new CServiceHandler();
		m_pServiceHandler1->Bind(this,EnumKeyServiceHandler1);
		m_pServiceHandler1->Init(pCfgServiceHandler);
	}

	CConfig *pCfgZC2;
	pCfgZC2 = m_pConfig->GetProperties(gc_sCfgIfZC2);
	if (0 != pCfgZC2 && !pCfgZC2->IsEmpty())
	{
		CRLog(E_NOTICE,"初始化连接点ZC2");
		m_pCpInterfaceZC2 = new CProtocolCpCli<CProcessInterfaceZC>();
		m_pCpInterfaceZC2->Bind(this,EnumKeyIfZC2);
		m_pCpInterfaceZC2->Init(pCfgZC2);

		CRLog(E_NOTICE,"初始化连接点ServiceHandler2");
		m_pServiceHandler2 = new CServiceHandler();
		m_pServiceHandler2->Bind(this,EnumKeyServiceHandler2);
		m_pServiceHandler2->Init(pCfgServiceHandler);
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
		
	CConfig *pCfgSvAgent;
	pCfgSvAgent = m_pConfig->GetProperties(gc_sCfgSvAgent);
	if (0 == pCfgSvAgent || pCfgSvAgent->IsEmpty())
	{
		pCfgSvAgent = m_pConfig;
	}
	m_pSvAgent = new CIfSvAgent();
	m_pSvAgent->Bind(this,EnumKeySvAgent);
	m_pSvAgent->Init(pCfgSvAgent);
	m_pSvAgent->SetObj(m_pNetMagModule, m_pCpInterfaceH1, m_pCpInterfaceH2);


	//初始化报文路由表
	InitRouterTbl();


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
int CSamplerCpMgr::Start()
{
	CRLog(E_NOTICE,"启动定时器管理器");
	CGessTimerMgrImp::Instance()->Start();
	CAbsTimerMgrWin32::Instance()->Start();

	if (0 != m_pSvAgent)
	{
		//CRLog(E_NOTICE,"启动SvAgent模块");
		m_pSvAgent->Start();
	}

	//启动网络监控理模块
	if (0 != m_pNetMagModule)
	{	
		CRLog(E_NOTICE,"启动网管代理模块");
		m_pNetMagModule->Start();
	}

	//if (0 != m_pQueueIo)
	//{
	//	CRLog(E_NOTICE,"启动XQueue");
	//	m_pQueueIo->Start();
	//}

	/*for(int i=0; i<2; ++i)
	{
		if(0  != m_vecQueueIo[i])
		    m_vecQueueIo[i]->Start();
	}*/

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"启动[XQUE%d]", nCount);
			(*it)->Start();
		}
	}



    // added by Jerry Lee, 2010-12-31
    if (0 != m_pQueHisData)
    {
        CRLog(E_NOTICE,"启动HisXQueue");
        m_pQueHisData->Start();
    }
    //
  

	if (0 != m_pTranslator)
	{
		CRLog(E_NOTICE,"启动Translator");
		m_pTranslator->Start();
	}

	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"启动ServiceHandler1");
		m_pServiceHandler1->Start();
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"启动ServiceHandler2");
		m_pServiceHandler2->Start();
	}


	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"启动连接点ZC1");
		m_pCpInterfaceZC1->Start();
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"启动连接点ZC2");
		m_pCpInterfaceZC2->Start();
	}

	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"启动连接点ZS");
		m_pCpInterfaceZS->Start();
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
	return 0;
}

//停止各连接点
void CSamplerCpMgr::Stop()
{
	//停止定时器管理器
	CRLog(E_NOTICE,"停止定时器管理器");
	CGessTimerMgrImp::Instance()->Stop();
	CAbsTimerMgrWin32::Instance()->Stop();

	if (0 != m_pTranslator)
	{
		CRLog(E_NOTICE,"停止Translator");
		m_pTranslator->Stop();
	}

	//if (0 != m_pQueueIo)
	//{
	//	CRLog(E_NOTICE,"停止XQueue");
	//	m_pQueueIo->Stop();
	//}

	//for(int i=0; i<2; ++i)
	//{
	//	if(0  != m_vecQueueIo[i])
	//	    m_vecQueueIo[i]->Stop();
	//}

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			CRLog(E_APPINFO,"停止[XQUE%d]", nCount);
			(*it)->Stop();
		}
	}


    // added by Jerry Lee, 2010-12-31
    if (0 != m_pQueHisData)
    {
        CRLog(E_NOTICE,"停止HisXQueue");
        m_pQueHisData->Stop();
    }
    //


	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"停止ServiceHandler1");
		m_pServiceHandler1->Stop();
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"停止ServiceHandler2");
		m_pServiceHandler2->Stop();
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"停止连接点ZC1");
		m_pCpInterfaceZC1->Stop();
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"停止连接点ZC2");
		m_pCpInterfaceZC2->Stop();
	}
	
	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"停止连接点ZS");
		m_pCpInterfaceZS->Stop();
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

	if (0 != m_pSvAgent)
	{
		m_pSvAgent->Stop();
	}
}

//结束清理
void CSamplerCpMgr::Finish()
{
	CGessTimerMgrImp::Instance()->Finish();
	CAbsTimerMgrWin32::Instance()->Finish();

	if (0 != m_pTranslator)
	{
		CRLog(E_NOTICE,"清理Translator");
		m_pTranslator->Finish();
		delete m_pTranslator;
		m_pTranslator = 0;
	}

	
	//if (0 != m_pQueueIo)
	//{
	//	CRLog(E_NOTICE,"清理XQueue");
	//	m_pQueueIo->Finish();
	//	delete m_pQueueIo;
	//	m_pQueueIo = 0;
	//}
	//for(int i=0; i<2; ++i)
	//{
	//	if(0  != m_vecQueueIo[i])
	//	{
	//		CRLog(E_NOTICE,"清理XQueue");
	//		m_vecQueueIo[i]->Finish();
	//		delete m_vecQueueIo[i];
	//		m_vecQueueIo[i]= 0;
	//	}
	//}

	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
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



    // added by Jerry Lee, 2010-12-31
    if (0 != m_pQueHisData)
    {
        CRLog(E_NOTICE,"清理XQueue");
        m_pQueHisData->Finish();
        delete m_pQueHisData;
        m_pQueHisData = 0;
    }
    //


	if (0 != m_pServiceHandler1)
	{
		CRLog(E_NOTICE,"清理ServiceHandler1");
		m_pServiceHandler1->Finish();
		delete m_pServiceHandler1;
		m_pServiceHandler1 = 0;
	}

	if (0 != m_pServiceHandler2)
	{
		CRLog(E_NOTICE,"清理ServiceHandler2");
		m_pServiceHandler2->Finish();
		delete m_pServiceHandler2;
		m_pServiceHandler2 = 0;
	}

	if (0 != m_pCpInterfaceZC1)
	{
		CRLog(E_NOTICE,"清理连接点ZC1");
		m_pCpInterfaceZC1->Finish();
		delete m_pCpInterfaceZC1;
		m_pCpInterfaceZC1 = 0;
	}

	if (0 != m_pCpInterfaceZC2)
	{
		CRLog(E_NOTICE,"清理连接点ZC2");
		m_pCpInterfaceZC2->Finish();
		delete m_pCpInterfaceZC2;
		m_pCpInterfaceZC2 = 0;
	}

	if (0 != m_pCpInterfaceZS)
	{
		CRLog(E_NOTICE,"清理连接点ZS");
		m_pCpInterfaceZS->Finish();	
		delete m_pCpInterfaceZS;
		m_pCpInterfaceZS = 0;
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

	if (0 != m_pSvAgent)
	{
		m_pSvAgent->Finish();
		delete m_pSvAgent;
		m_pSvAgent = 0;
	}
	
	
	CLogger::Instance()->Finish();
	delete m_pConfig;
	m_pConfig = 0;

	CNetMgr::Instance()->NmFinish();
	CNetMgr::Instance()->UnRegisterModule(&m_oNmoModule);
	
	//
	m_oMemShareAlive.UnMap();
}

//启动线程
int CSamplerCpMgr::StartMe()
{
	//telnet 报文处理连接点绑定
	m_oCpCmdHandler.Bind(this,EnumKeyCmdHandler);
	//网络日志回调对象绑定
	m_oNetLogHost.Bind(this);
	//网络日志处理线程绑定
	m_oNetLogThread.Bind(this);
	//命令行处理线程绑定
	m_oCmdLineThread.Bind(this);
	//
	m_oStaticsPipeThread.Bind(this);

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

	//
	m_oStaticsPipeThread.BeginThread();

	//启动网络日志处理线程
	m_oNetLogThread.BeginThread();
	return 0;
}

//停止线程
void CSamplerCpMgr::StopMe()
{
	m_oNetLogThread.EndThread();
	//m_oCmdLineThread.EndThread();

	m_oStaticsPipeThread.EndThread();

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
}

//特殊命令报文
int CSamplerCpMgr::HandleCmdSpecial(const string& sCmdID, CPacket &pkt)
{
	int nRtn = -1;
	try
	{
		// added by Ben 2011-05-29 处理退出控制命令
		if (strutils::ToHexString<unsigned int>(YL_QUITMSG) == sCmdID)
		{
			CSamplerPacket &sPkt = dynamic_cast<CSamplerPacket &>( pkt);
			unsigned int uiNodeidDest = 0;
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(sPkt.GetMsg());
			msg.GetField(MSG_BODY_NODEID, uiNodeidDest);

			if (uiNodeidDest == m_uiNodeID)
			{
				m_bStop = true;
				m_deqCondMutex.Signal();
			}
			nRtn = 0;
		}
		// end add
		else if (strutils::ToHexString<unsigned int>(YL_SYNC_TIME) == sCmdID)
		{
			CSamplerPacket &sPkt = dynamic_cast<CSamplerPacket &>( pkt);
			unsigned int uiNodeidDest = 0;
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(sPkt.GetMsg());
			msg.GetField(MSG_BODY_NODEID, uiNodeidDest);

			if (uiNodeidDest == m_uiNodeID)
			{
				unsigned int uiDate = 0;
				unsigned int uiTime = 0;
				msg.GetField(MSG_DATE, uiDate);
				msg.GetField(MSG_TIME, uiTime);

				SYSTEMTIME st;
				st.wYear = uiDate / 100000;
				st.wMonth = (uiDate % 100000) / 1000;
				st.wDay = (uiDate % 1000) / 10;
				st.wDayOfWeek = uiDate % 10;

				st.wHour = uiTime / 10000000;
				st.wMinute = (uiTime % 10000000) / 100000;
				st.wSecond = (uiTime % 100000) / 1000;
				st.wMilliseconds = (uiTime % 1000) / 2;
				::SetLocalTime(&st);
			}
			nRtn = 0;
		}
		// end add
		return nRtn;
	}
	catch(...)
	{
		return -1;
	}
}

//报文转发引擎 返回值-2表示无路由
int CSamplerCpMgr::Forward(CPacket &pkt,const unsigned long& ulKey)
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

		//特殊命令处理
		if (0 == HandleCmdSpecial(sCmdID, pkt))
			return 0;

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
int CSamplerCpMgr::Run()
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
string CSamplerCpMgr::OnCmd(const string& sCmdLine, const vector<string>& vecPara)
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
		sRtn += "Sampler->";
		return sRtn;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		string sRtn = "\r\nSampler->";
		return sRtn;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		string sRtn = "\r\nSampler->";
		return sRtn;
	}
}

//telnet终端命令处理
int CSamplerCpMgr::OnPacketCmd(CPacket& pkt)
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
			sRsp += "Sampler->";
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

				sRsp += "Sampler->";
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

				sRsp += "Sampler->";
			}
			else
			{
				sRsp = "Parameter err!";
				sRsp += "\r\n";
				sRsp += "Sampler->";
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
void CSamplerCpMgr::OnNetLogMsg(const string& sMsg)
{
	m_oNetLogThread.Enque(sMsg);
}

//日志发送
int CSamplerCpMgr::HandleNetLogMsg(const string & sNetLogMsg)
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
int CSamplerCpMgr::HandleCmdLine(string& sIn)
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

int CSamplerCpMgr::HandleStatics(const char* pBuf,unsigned int uiLen)
{
	if (0 == pBuf || 0 == uiLen)
		return -1;

	string sVal;
	sVal.assign(pBuf,uiLen);

	string sFlag = " by:";
	sFlag += ToString<unsigned int>(m_uiNodeID);
	sVal.append(sFlag);
	NotifyEvent(sVal);

	//string sVal;
	//sVal.assign(pBuf,uiLen);
	//NotifyEvent

	//CSamplerPacket oPacketHello(YL_HELLO);
	//CMessage &  msg = oPacketHello.GetMsg();

	//string sVal;
	//sVal.assign(pBuf,uiLen);

	//string sFlag = " by:";
	//sFlag += m_sNodeID;
	//sVal.append(sFlag);
	//msg.SetBinaryField(MSG_HELLO_CONTENT,sVal);

	//if (0 != m_pCpInterfaceZC1)
	//{
	//	m_pCpInterfaceZC1->SendPacket(oPacketHello);
	//}
	//else if (0 != m_pCpInterfaceZC2)
	//{
	//	m_pCpInterfaceZC2->SendPacket(oPacketHello);
	//}
	//else
	//{
	//	//nothing
	//}
	return 0;
}

//K接口心跳定时器回调接口
int CSamplerCpMgr::OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan)
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
		CNMO oNmo;
		
		//kenny加的循环
		for(int i=0; i<2; ++i)
	    {


			if (0 != m_vecQueueIo[i])
			{
				if (0 == /*m_pQueueIo */m_vecQueueIo[i]->GetBlockInf(nFree, nUsed, nTotal))			
				{
					oNmo.m_sOid=gc_sMemQueFree;
					oNmo.m_sOidIns = gc_sMemQueFree+".XQUE.QUOTATION";
					oNmo.m_sValue=ToString(nFree);
					vNmo.push_back(oNmo);

					oNmo.m_sOid=gc_sMemQueUsed;
					oNmo.m_sOidIns = gc_sMemQueUsed+".XQUE.QUOTATION";
					oNmo.m_sValue=ToString(nUsed);
					vNmo.push_back(oNmo);


					oNmo.m_sOid=gc_sMemQueTotal;
					oNmo.m_sOidIns = gc_sMemQueTotal+".XQUE.QUOTATION";
					oNmo.m_sValue=ToString(nTotal);
					vNmo.push_back(oNmo);

					CNetMgr::Instance()->Report(vNmo);
				}
			}
		}

		if (0 != m_pQueHisData)
		{
			nFree = 0;
			nUsed = 0;
			nTotal = 0;

			if (0 == m_pQueHisData->GetBlockInf(nFree, nUsed, nTotal))			
			{
				oNmo.m_sOid=gc_sMemQueFree;
				oNmo.m_sOidIns = gc_sMemQueFree+".XQUE.HISDATA";
				oNmo.m_sValue=ToString(nFree);
				vNmo.push_back(oNmo);

				oNmo.m_sOid=gc_sMemQueUsed;
				oNmo.m_sOidIns = gc_sMemQueUsed+".XQUE.HISDATA";
				oNmo.m_sValue=ToString(nUsed);
				vNmo.push_back(oNmo);

				oNmo.m_sOid=gc_sMemQueTotal;
				oNmo.m_sOidIns = gc_sMemQueTotal+".XQUE.HISDATA";
				oNmo.m_sValue=ToString(nTotal);
				vNmo.push_back(oNmo);

				CNetMgr::Instance()->Report(vNmo);
			}
		}
	}
	guiTimerCount++;
	return 0;
}

//定时重启定时器回调接口
int CSamplerCpMgr::OnResetTimeout(const string& sTmKey)
{
	//事件通知
	string sEvtContent = "定时时间到,稍后开始重启!";

	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	m_bStop = true;
	m_deqCondMutex.Signal();
	return 0;
}

//帮助信息
string CSamplerCpMgr::OnCmdLineHelp(const string& sCmdLine, const vector<string>& vecPara)
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
	sRtn += "Sampler->";

	return sRtn;
}

//quit命令处理
string CSamplerCpMgr::OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara)
{
	//事件通知
	string sEvtContent = "命令行发出Quit退出指令,大约3秒后退出!";
	CRLog(E_NOTICE,sEvtContent.c_str());
	NotifyEvent(sEvtContent);

	CancelOrder();
	msleep(3);

	m_bStop = true;
	m_deqCondMutex.Signal();

	string sRtn = "Sampler->";
	return sRtn;
}

string CSamplerCpMgr::OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara)
{
	string sRtn = CSelectorIo::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += CSelectorListen::Instance()->ToString();
	sRtn += "\r\n";
	sRtn += "Sampler->";
	return sRtn;
}

//显示内存
string CSamplerCpMgr::OnCmdLineMem(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Sampler->\r\n";

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

int CSamplerCpMgr::HandleConsolMsg(unsigned int uiMsg)
{
	
	//事件通知
	string sEvtContent = "";
	switch (uiMsg)
	{
	case CTRL_CLOSE_EVENT:
		sEvtContent = "报盘机控制台窗口被强制关闭,现在退出应用!";
		break;
	case CTRL_SHUTDOWN_EVENT:
		sEvtContent = "报盘机计算机关机,现在退出应用!";
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

void CSamplerCpMgr::NotifyEvent(const string& sEvt)
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

string CSamplerCpMgr::OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara)
{
	//事件通知
	string sEvtContent = "事件测试!";
	NotifyEvent(sEvtContent);

	string sRtn = "Sampler->";
	return sRtn;
}

//队列长度
string CSamplerCpMgr::OnCmdLineQue(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Sampler->";
	
	return sRtn;
}

int CSamplerCpMgr::CancelOrder()
{
	CMessageImpl oMsgUnsub;
	
	unsigned int uiVal = 0xFFFFFFFF;
	string sItems;
	sItems.assign((char*)(&uiVal),sizeof(unsigned int));
	oMsgUnsub.SetBinaryField(MSG_SUBSCRIP_RECS,sItems);

	CSamplerPacket oPktUnsub(oMsgUnsub,YL_UNSUBSCRIP);
	if (0 != m_pCpInterfaceZC1)
	{
		m_pCpInterfaceZC1->SendPacket(oPktUnsub);
	}

	if (0 != m_pCpInterfaceZC2)
	{
		m_pCpInterfaceZC2->SendPacket(oPktUnsub);		
	}

	return 0;
}

int CSamplerCpMgr::ToXQueue(QUOTATION& stQuotation)
{
	//统计时延
	//unsigned int uiTime = stQuotation.m_uiTime/1000;
	//CGessTime oTimeQuo(uiTime/10000, (uiTime % 10000)/100, uiTime % 100);
	//int nDelay = oTimeQuo.IntervalToNow();
	SYSTEMTIME st;
	::GetLocalTime(&st);
	unsigned int uiQuoTime = stQuotation.m_uiTime/1000;
	unsigned int uiHour = uiQuoTime/10000;
	unsigned int uiMin = (uiQuoTime%10000)/100;
	unsigned int uiSec = uiQuoTime%100;

	int nDelay = st.wHour*3600 + st.wMinute * 60 + st.wSecond - uiHour*3600 - uiMin*60 - uiSec;
	if (nDelay > 600 || nDelay < -600)
	{
		if(m_bTimeDelayFilter)
		{
			CRLog(E_DEBUG, "(%02d:%02d:%02d-%02d:%02d:%02d)[%s]%s,行情时间过期:%u %06u 延时:%d, 价格:%u", st.wHour,st.wMinute,st.wSecond,uiHour,uiMin,uiSec,stQuotation.m_CodeInfo.m_acCode, stQuotation.m_CodeInfo.m_acName,stQuotation.m_uiDate, stQuotation.m_uiTime/1000, nDelay, stQuotation.m_uiLast);
		    return 0;
		}
	}

	if (nDelay > 60)
	{
		CRLog(E_DEBUG, "(%02d:%02d:%02d-%02d:%02d:%02d)[%s]%s,行情日期/时间:%u %06u 较大延时:%d, 价格:%u", st.wHour,st.wMinute,st.wSecond,uiHour,uiMin,uiSec,stQuotation.m_CodeInfo.m_acCode, stQuotation.m_CodeInfo.m_acName,stQuotation.m_uiDate, stQuotation.m_uiTime/1000, nDelay, stQuotation.m_uiLast);
	}

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

	//分发给相应队列处理

	//if (0 != m_pQueueIo)
	//{
	//	m_pQueueIo->Enque(stQuotation);
	//	m_uiFwdTotal++;
	//}

	//for(int i=0;  i<2;  ++i)
	//{	
	//	if (0 != m_vecQueueIo[i])
	//	{
	//		m_vecQueueIo[i]->Enque(stQuotation);
	//		m_uiFwdTotal++;
	//	}

	//}

	//分发给相应队列处理
	int nCount = 0;
	for (vector< CXQueueIo<QUOTATION>*  >::iterator it = m_vecQueueIo.begin(); it != m_vecQueueIo.end(); ++it)
	{
		nCount++;
		if (0 != *it)
		{
			(*it)->Enque(stQuotation);

			m_uiFwdTotal++;
		}
	}


	return 0;
}

int CSamplerCpMgr::ToHisDataQueue(CSamplerPacket& oPktSrc)
{
    CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPktSrc.GetMsg());

    string strHisData;

    //printf("收到历史数据包\n");

    if (0 != msg.GetBinaryField(MSG_HISTORY_DATA, strHisData))
    {
        CRLog(E_APPINFO,"读取历史数据错误!");
        return -1;
    }

    // 计算buffer的数量
    int nSize = strHisData.length()/sizeof(HistoryDataBuf); 
    nSize += (strHisData.length()%sizeof(HistoryDataBuf))?1:0;


#if 0
    FixedHistoryData  *pFHA = (FixedHistoryData *)strHisData.data();
    ofstream out;
    string strFileName = "F:\\"; 
    strFileName += pFHA->header.productCode;
    strFileName += ".day";
    out.open(strFileName.c_str(), ios::out|ios::binary|ios::app);
    out.write(pFHA->content, pFHA->header.length);
    out.close();
    out.clear();
#endif

    // 将Buffer写入到内存队列
    HistoryDataBuf hdb;

    for (int i = 0; i < nSize; i++)
    {
        memset(&hdb, 0, sizeof(hdb));

        memcpy(&hdb, strHisData.data()+i*sizeof(HistoryDataBuf), 
            min(sizeof(HistoryDataBuf), strHisData.length()-i*sizeof(HistoryDataBuf)));

        m_pQueHisData->Enque(hdb);
    }

    return 0;
}

int CSamplerCpMgr::ToTickDataQueue(CSamplerPacket& oPktSrc)
{
    CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPktSrc.GetMsg());

    string strTickData;

    //printf("收到tick数据包\n");

    if (0 != msg.GetBinaryField(MSG_TICK_DATA, strTickData))
    {
        CRLog(E_APPINFO,"读取tick数据错误!");
        return -1;
    }

    // 计算buffer的数量
    int nSize = strTickData.length()/sizeof(HistoryDataBuf); 
    nSize += (strTickData.length()%sizeof(HistoryDataBuf))?1:0;


#if 0
    FixedTickData  *pFHA = (FixedTickData *)strTickData.data();
    ofstream out;
    string strFileName = "F:\\"; 
    strFileName += "tick.";
    strFileName += pFHA->header.productCode;
    out.open(strFileName.c_str(), ios::out|ios::binary|ios::app);
    out.write(pFHA->content, pFHA->header.length);
    out.close();
    out.clear();
#endif

    // 将Buffer写入到内存队列
    HistoryDataBuf hdb;

    for (int i = 0; i < nSize; i++)
    {
        memset(&hdb, 0, sizeof(hdb));

        memcpy(&hdb, strTickData.data()+i*sizeof(HistoryDataBuf), 
            min(sizeof(HistoryDataBuf), strTickData.length()-i*sizeof(HistoryDataBuf)));

        m_pQueHisData->Enque(hdb);
    }

    return 0;
}

int CSamplerCpMgr::ToInfoDataQueue(CSamplerPacket& oPktSrc)
{
    CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPktSrc.GetMsg());

    string strInfoData;

    if (0 != msg.GetBinaryField(MSG_INFO_DATA, strInfoData))
    {
        CRLog(E_APPINFO,"读取Info数据错误!");
        return -1;
    }

    // 计算buffer的数量
    int nSize = strInfoData.length()/sizeof(HistoryDataBuf); 
    nSize += (strInfoData.length()%sizeof(HistoryDataBuf))?1:0;


    // 将Buffer写入到内存队列
    HistoryDataBuf hdb;

    for (int i = 0; i < nSize; i++)
    {
        memset(&hdb, 0, sizeof(hdb));

        memcpy(&hdb, strInfoData.data()+i*sizeof(HistoryDataBuf), 
            min(sizeof(HistoryDataBuf), strInfoData.length()-i*sizeof(HistoryDataBuf)));

        m_pQueHisData->Enque(hdb);
    }

    return 0;
}

//系统监控采集
int CSamplerCpMgr::Query(CNMO& oNmo)
{
	int nRtn = -1;
	oNmo.m_nQuality=gc_nQuolityGood;

	if(oNmo.m_sOidIns==(mibQueNum+"."+"Translator队列"))
	{
		if (0 != m_pTranslator)
		{
			oNmo.m_sValue=ToString<unsigned int>(m_pTranslator->QueueLen());
			nRtn = 0;
		}
		return nRtn;
	}
	else if (oNmo.m_sOid == gc_sDelayMin)
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
	for(int i=0;  i<2; ++i)
	{
		if (0 != /*m_pQueueIo*/m_vecQueueIo[i])
		{
			if(oNmo.m_sOidIns==mibQueNum+"."+ "XQUE.QUOTATION.写缓存队列")
			{
				oNmo.m_sValue=ToString<unsigned int>(m_vecQueueIo[i]-> QueueLen());
				nRtn = 0;
				return nRtn;
			}
			else if (oNmo.m_sOidIns==gc_sMemQueFree+"."+ "XQUE.QUOTATION")
			{
				if (0 == m_vecQueueIo[i]->GetBlockInf(nFree, nUsed, nTotal))
				{
					oNmo.m_sValue=ToString<unsigned int>(nFree);
					nRtn = 0;
				}
				return nRtn;
			}
			else if (oNmo.m_sOidIns==gc_sMemQueUsed+"."+ "XQUE.QUOTATION")
			{
				if (0 == m_vecQueueIo[i]->GetBlockInf(nFree, nUsed, nTotal))
				{
					oNmo.m_sValue=ToString<unsigned int>(nUsed);
					nRtn = 0;
				}
				return nRtn;
			}
			else if (oNmo.m_sOidIns==gc_sMemQueTotal+"."+ "XQUE.QUOTATION")
			{
				if (0 == m_vecQueueIo[i]->GetBlockInf(nFree, nUsed, nTotal))
				{
					oNmo.m_sValue=ToString<unsigned int>(nTotal);
					nRtn = 0;
				}
				return nRtn;
			}
		}
		
	}
	

	if (0 != m_pQueHisData)
	{
		if(oNmo.m_sOidIns==mibQueNum+"."+ "XQUE.HISDATA.写缓存队列")
		{
			oNmo.m_sValue=ToString<unsigned int>(m_pQueHisData-> QueueLen());
			nRtn = 0;
			return nRtn;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueFree+"."+ "XQUE.HISDATA")
		{
			if (0 == m_pQueHisData->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString<unsigned int>(nFree);
				nRtn = 0;
			}
			return nRtn;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueUsed+"."+ "XQUE.HISDATA")
		{
			if (0 == m_pQueHisData->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString<unsigned int>(nUsed);
				nRtn = 0;
			}
			return nRtn;
		}
		else if (oNmo.m_sOidIns==gc_sMemQueTotal+"."+ "XQUE.HISDATA")
		{
			if (0 == m_pQueHisData->GetBlockInf(nFree, nUsed, nTotal))
			{
				oNmo.m_sValue=ToString<unsigned int>(nTotal);
				nRtn = 0;
			}
			return nRtn;
		}
	}
	return nRtn;
}