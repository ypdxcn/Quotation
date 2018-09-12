#ifndef _DATA_PROVIDER_CP_MGR_H
#define _DATA_PROVIDER_CP_MGR_H
#include "netlogdev.h"
#include "IpcPacket.h"
#include "Comm.h"
#include "WorkThreadNm.h"
#include "SamplerPacket.h"
#include "XQueueIo.h"
#include <vector>
#include <string>
#include <iostream>
#include "ConfigImpl.h"
#include "MemShareAlive.h"
#include "PktBinBlock.h"
#include "BasicInfoMemFile.h"

using namespace std;

//连接点key常量
typedef enum tagEnumKey
{
	EnumKeyDeliverMgr,
	EnumKeyServiceHandlerSvr,
	EnumKeyIfZS,
	EnumKeyServiceHandlerCln1,
	EnumKeyIfZC1,
	EnumKeyIfH1,
	EnumKeyIfH2,
	EnumKeyIfCmd,
	EnumKeyCmdHandler,
	EnumNetMagModule,
	EnumKeySelfIpc,
	EnumKeyUnknown
} EnumKeyIf;

//各个连接点配置项名
const string gc_sCfgWriter = "XQUEUE_SVR";
const string gc_sCfgReader = "XQUEUE_QUO";
const string gc_sCfgDeliver = "IFDELIVER";
const string gc_sCfgService = "IFSERVICE";
const string gc_sCfgHisData = "IFHISDATA";
const string gc_sCfgHisDataFx = "IFHISDATAFX";
const string gc_sCfgIfZS = "IFZS";
const string gc_sCfgIfZC1 = "IFZC1";
const string gc_sCfgIfH1 = "IFH1";
const string gc_sCfgIfH2 = "IFH2";
const string gc_sCfgIfCmd = "IFCMD";
const string gc_sCfgNetMagModule = "net_mgr";

//缺省命令字匹配
const string gc_sDefaultCmdID = "#";


//递延交收行情  
typedef struct
{
	char			instID[16];				//合约代码
	unsigned int	bidLot;					//买量
	unsigned int	askLot;					//卖量
	unsigned int	midBidLot;				//中立仓买量
	unsigned int	midAskLot;				//中立仓卖量
} DeliveryQuotation, *PDeliveryQuotation;

//重启通知
typedef struct
{
	char  StartDate[12];				//启动日期
	char  StartTime[12];					//启动时间
} START_NTF,*PSTART_NTF;

//基础信息
typedef struct
{
	char  memberID[6];				//会员ID
	char  traderID[10];					//交易员ID
	char  exchDate[12];
	char  ts[16];
} BASIC_INFO,*PBASIC_INFO;


//合约交易状态信息  
typedef struct
{
	char  instID[12];				//合约代码
	char  marketID;					//市场
	char  tradeState;				//合约交易状态
} INST_STATE,*PINST_STATE;

typedef struct
{
	char  ts[12];					//时间戳
} LOGIN,*PLOGIN;

#define	PKT_INST_STATE				0x00000001
#define	PKT_QUOTATION				0x00000002
#define	PKT_DELIVERY_QUOTATION		0x00000003

#define	PKT_START_NTF				0x0000000A
#define	PKT_BASIC_INFO				0x0000000B

#define	PKT_LOGIN					0x80000001


class CGessTimerMgr;
class CConfigImpl;
class CNetMgrModule;
class CHisDataHandler;
class CHisDataHandlerFx;
class CDeliverMgr;
class CProviderCpMgr:public CProtocolCpMgr, public CXQueueCallback<CBinBlockPkt>
{
private:
	int Query(CNMO& oNmo) ;
	class CCpMgrNm: public CNmoModule
	{
	public:
		CCpMgrNm():m_pParent(0){}
		virtual ~CCpMgrNm(){}
		void Bind(CProviderCpMgr* pParent){m_pParent = pParent;}
		//单个查询接口
		int Query(CNMO& oNmo) const
		{
			if (0 != m_pParent)
				return m_pParent->Query(oNmo);
			return -1;
		}

		//批量查询接口
		int Query(vector< CNMO > & vNmo) const
		{
			for (vector< CNMO >::iterator it = vNmo.begin(); it != vNmo.end(); ++it)
			{
				Query(*it);
			}
			return 0;
		}

		//控制接口
		int Operate(const string &sOid, int nflag, const string &sDstValue, const string &sParamer) {return -1;}
	private:
		CProviderCpMgr * m_pParent;
	};

	//命令行处理线程
	class CCommandLineThread :public CWorkThreadNm
	{
	public:
		CCommandLineThread():m_pParent(0){}
		virtual ~CCommandLineThread(){}
		void Bind(CProviderCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			string sIn("");	
			cout << "Provider->";
			try
			{
				while(!m_bEndThread)
				{
					try
					{
						if (0 != m_pParent)
						{
							//等待键盘输入,可能一直阻塞,直到键盘输入或主线程/进程退出
							m_pParent->HandleCmdLine(sIn);
						}
						else
						{
							msleep(2);
						}
					}
					catch(...)
					{
						CRLog(E_ERROR,"Unknown exception!");
						msleep(1);
					}
				}

				CRLog(E_SYSINFO,"Provider_CmdLine_Thread exit!");
				return 0;
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
		int End()
		{
			return 0;
		}
		//主动类线程状态是否需要被网管
		bool IsNetManaged(string& sKeyName)
		{
			sKeyName = "命令行处理线程";
			return true;
		}
	private:
		CProviderCpMgr* m_pParent;
	};

	//网络日志类消息处理线程
	class CNetLogThread :public CWorkThreadNm
	{
	public:
		CNetLogThread():m_pParent(0){}
		virtual ~CNetLogThread(){}
		void Bind(CProviderCpMgr* p){m_pParent = p;}
		int Enque(const string& sMsg)
		{
			m_deqCondMutex.Lock();
			m_deqLog.push_back(sMsg);
			m_deqCondMutex.Unlock();
			m_deqCondMutex.Signal();
			return 0;
		}
	private:
		int ThreadEntry()
		{
			try
			{
				while(!m_bEndThread)
				{
					m_deqCondMutex.Lock();
					while(m_deqLog.empty() && !m_bEndThread)
						m_deqCondMutex.Wait();
					
					if (m_bEndThread)
					{
						m_deqLog.clear();
						m_deqCondMutex.Unlock();
						break;
					}

					if ( !m_deqLog.empty())
					{
						string sMsg = m_deqLog.front();
						m_deqLog.pop_front();
						m_deqCondMutex.Unlock();
						
						if (0 == m_pParent)
							continue;

						try
						{
							m_pParent->HandleNetLogMsg(sMsg);
						}
						catch(std::exception e)
						{
							CRLog(E_ERROR,"exception:%s!",e.what());
						}
						catch(...)
						{
							CRLog(E_ERROR,"Unknown exception!");
						}
					}
				}

				CRLog(E_SYSINFO,"Provider_NetLog Thread exit!");
				return 0;
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
		int End()
		{
			m_deqCondMutex.Lock();
			m_deqCondMutex.Signal();
			m_deqCondMutex.Unlock();
			Wait();
			return 0;
		}
		//主动类线程状态是否需要被网管
		bool IsNetManaged(string& sKeyName)
		{
			sKeyName = "网络日志信息线程";
			return true;
		}
	private:
		CProviderCpMgr* m_pParent;
		//日志消息队列
		std::deque<string> m_deqLog;
		CCondMutex	m_deqCondMutex;
	};

	//telnet 命令行处理连接点
	class CConnectPointCmd:public CConnectPointAsyn
	{
	public:
		CConnectPointCmd():m_pMgr(0),m_ulKey(EnumKeyUnknown){}
		~CConnectPointCmd(){}
		int Init(CConfig* pConfig){return 0;}
		int Start(){return 0;}
		void Stop(){}
		void Finish(){}
		int OnRecvPacket(CPacket &GessPacket){return 0;}
		void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
		{
			m_pMgr=dynamic_cast<CProviderCpMgr*>(pCpMgr); 
			m_ulKey=ulKey;
		}
		int SendPacket(CPacket &GessPacket)
		{
			if (0==m_pMgr)
				return -1;
			return m_pMgr->OnPacketCmd(GessPacket);
		}
	private:
		CProviderCpMgr* m_pMgr;
		unsigned long m_ulKey;
	};

	//日志主机
	class CNetLogHost:public CSubscriber
	{
	public:
		CNetLogHost():m_pMgr(0){}
		virtual ~CNetLogHost(){}
		void Bind(CProviderCpMgr* p){m_pMgr = p;}
		void OnNotify(const string& sMsg)
		{
			if (0 != m_pMgr)
				m_pMgr->OnNetLogMsg(sMsg);
		}
	private:
		CProviderCpMgr* m_pMgr;
	};

	//dog心跳定时器
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CProviderCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CProviderCpMgr* m_pParent;
	};

	//定时重启定时器
	class CResetTimer: public CGessAbsTimer
	{
	public:
		CResetTimer():m_pParent(0){}
		~CResetTimer(void){}
		void Bind(CProviderCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& sKey)
		{
			if (0 != m_pParent)
				return m_pParent->OnResetTimeout(sKey);
			return -1;
		}
		void TimerCanceled(const string& ulKey){}
	private:
		CProviderCpMgr* m_pParent;
	};
public:
	CProviderCpMgr();
	virtual ~CProviderCpMgr();
	
	int OnConnect(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	int OnAccept(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);	
	int OnClose(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);
	int OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	//int OnLogout
	int Forward(CPacket &GessPacket,const unsigned long& ulKey);

	int Init(const string& sProcName);
	void Finish();
	int Start();
	void Stop();
	int Run();

	int HandleConsolMsg(unsigned int uiMsg);

	int OnXQueuePkt(CBinBlockPkt& oBlockPkt);
	int ToHisData(const string& sQuotation);
	int ToHisDataFx(const string& sQuotation);
private:

	//定义成员函数指针
	typedef string (CProviderCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
	//报文命令字与报文处理成员函数映射结构
	typedef struct tagCmdLine2Api
	{
		string sCmdName;					//CmdName
		string sCmdAbbr;					//命令缩写
		MFP_CmdHandleApi pMemberFunc;		//报文处理函数指针
		string sHelp;						//命令说明
	} CmdLine2Api;
	//命令行命令字与命令处理成员函数映射表
	static CmdLine2Api m_CmdLine2Api[];


	//源接口+报文命令字与对应路由接口映射结构
	typedef struct tagIfRouterCfg
	{
		CConnectPointAsyn* pCp;
		unsigned long ulIfTo;
		unsigned long ulIfFrom;
		string sCmdID;
	} IfRouterCfg;
	//源接口+报文命令字与对应路由接口映射关系配置表
	static IfRouterCfg m_tblIfRouterCfg[];

	//路由点
	typedef multimap<string,CConnectPointAsyn*> MMAP_CP;
	typedef MMAP_CP::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_CP;
	typedef struct tagIfRouterPoint
	{
		unsigned long ulIfFrom;
		MMAP_CP  mmapCmds;
	} IfRouterPoint;
	//内存路由表
	IfRouterPoint m_tblIfRouter[EnumKeyUnknown];

	int InitRouterTbl();
	
protected:

private:
	int HandleNetLogMsg(const string & sNetLogMsg);
	int HandleCmdLine(string& sIn);

	int OnResetTimeout(const string& sTmKey);
	int OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan);
	int OnPacketSelfIpc(CPacket& GessPacket);
	

	//telnet 命令行连接点回调接口
	int OnPacketCmd(CPacket& GessPacket);		
	//日志回调接口
	void OnNetLogMsg(const string& sMsg);

	//网管事件通知
	void NotifyEvent(const string& sEvt);	
private:
	std::string				m_sProcName;			//进程名称
	
	CConnectPointAsyn*		m_pCpInterfaceCmd;		//telnet服务接口
	CConnectPointAsyn*		m_pCpInterfaceH1;		//系统监控H1接口
	CConnectPointAsyn*		m_pCpInterfaceH2;		//系统监控H2接口
	CConnectPointAsyn*		m_pCpInterfaceZS;		//采集器与接收器接口	
	CConnectPointAsyn*		m_pCpInterfaceZC1;		//采集器与接收器接口
	CHisDataHandler*		m_pHisDataHandler;		//
	CHisDataHandlerFx*		m_pHisDataHandlerFx;		//
	CConnectPointAsyn*		m_pServiceHandlerSvr;		//
	CConnectPointAsyn*		m_pServiceHandlerCln1;		//
	CDeliverMgr*			m_pDeliverMgr;          //行情发布管理者接口

	CNetMgrModule*			m_pNetMagModule;		//网管代理
	CConnectPointCmd		m_oCpCmdHandler;		//Telnet命令行处理

	//看门狗共享内存
	CMemShareAlive			m_oMemShareAlive;

	//命令行处理线程
	CCommandLineThread		m_oCmdLineThread;
	//网络日志处理线程
	CNetLogThread			m_oNetLogThread;

	//日志主机
	CNetLogHost				m_oNetLogHost;


	//远程Telnet命令行终端订阅列表
	map<string,string>  m_deqTelnets;
	CGessMutex			m_csTelnets;
	
	//监控
	CCpMgrNm	m_oNmoModule;

	CCondMutex	m_deqCondMutex;

	//定时器管理器
	CGessTimerMgr* m_pGessTimerMgr;
	//dog定时器
	CDogTimer m_oIfkTimer;

	//定时重启定时器
	CResetTimer m_oResetTimer;
	//定时重启时间点配置
	vector<CGessTime> m_vResetTime;	

	//接口连接点连接数量
	int m_nConNumIf[EnumKeyUnknown];
	CGessMutex	m_csConNum;

	unsigned int			m_uiNodeID;
	unsigned int			m_uiNodeType;
	CConfigImpl*			m_pConfig;
	volatile bool			m_bStop;

	//行情日期模式
	int			  m_nQuoDateMode;
	//日期超期是否丢弃
	int			  m_nExpDateDiscard;
	//行情时间模式
	int			  m_nQuoTmMode;
	//时延调整
	int			  m_nDiff;
	//允许最大时延，超过则丢弃
	int			  m_nDelayPermit;
	//超过一定时延需要打印
	int			  m_nDelayPrint;

	//合约状态基本信息
	CBasicInfMemFile		m_oBasicInf;
	//合约ID转换
	CConfigImpl				m_oNameConvertFile;
	map<string, string>		m_mapNamePair;

	
	CXQueueIo<CBinBlockPkt>*	m_pReader;
	CXQueueIo<CBinBlockPkt>*	m_pIoServiceWriter;
private:
	void MemSendLogin(); 
	int HandleStartNtf(const char* pBuf, unsigned int uiLen);
	int HandleBasicInfo(const char* pBuf, unsigned int uiLen);
	int HandleInstState(const char* pBuf, unsigned int uiLen);
	int HandleQuotation(const char* pBuf, unsigned int uiLen);
	int HandleDeliveryQuotation(const char* pBuf, unsigned int uiLen);
	

	//合约ID转换
	void ConvertInstID(string& sInstID);
private:
	//命令行命令分发
	string OnCmd(const string& sCmdLine, const vector<string>& vecPara);

	string OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineMem(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineHelp(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara);
	string OnCmdLineQue(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineReplayQuotation(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineLoad(const string& sCmd, const vector<string>& vecPara);
private:

};
#endif