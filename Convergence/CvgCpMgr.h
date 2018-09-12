#ifndef _CONVERGENCE_CP_MGR_H
#define _CONVERGENCE_CP_MGR_H

#include "netlogdev.h"
#include "IpcPacket.h"
#include "Comm.h"
#include "WorkThreadNm.h"
#include <string>
#include <iostream>
#include "InfoPublisher.h"
#include "MemShareAlive.h"

using namespace std;

//连接点key常量
typedef enum tagEnumKey
{
	EnumKeyHisDataHandler,
	EnumKeyDeliverMgr,	
	EnumKeyServiceHandlerSvr,
	EnumKeyIfZS,
	EnumKeyServiceHandler1,
	EnumKeyServiceHandler2,
	EnumKeyServiceHandler3,
	EnumKeyServiceHandler4,
	EnumKeyServiceHandler5,
	EnumKeyServiceHandler6,
	EnumKeyServiceHandler7,
	EnumKeyServiceHandler8,
	EnumKeyIfZC1,
	EnumKeyIfZC2,
	EnumKeyIfZC3,
	EnumKeyIfZC4,
	EnumKeyIfZC5,
	EnumKeyIfZC6,
	EnumKeyIfZC7,
	EnumKeyIfZC8,
	EnumKeyIfH1,
	EnumKeyIfH2,
	EnumKeyIfCmd,
	EnumKeyCmdHandler,
	EnumNetMagModule,
	EnumKeySelfIpc,
	EnumKeySvAgent,
	EnumKeyUnknown
} EnumKeyIf;

//各个连接点配置项名
//const string gc_sCfgService1 = "IFSERVICE1";
//const string gc_sCfgService2 = "IFSERVICE2";
const string gc_sCfgDeliver = "IFDELIVER";
const string gc_sCfgService = "IFSERVICE";
const string gc_sCfgHisData = "IFHISDATA";
const string gc_sCfgIfZS = "IFZS";
const string gc_sCfgIfZC1 = "IFZC1";
const string gc_sCfgIfZC2 = "IFZC2";
const string gc_sCfgIfZC3 = "IFZC3";
const string gc_sCfgIfZC4 = "IFZC4";
const string gc_sCfgIfZC5 = "IFZC5";
const string gc_sCfgIfZC6 = "IFZC6";
const string gc_sCfgIfZC7 = "IFZC7";
const string gc_sCfgIfZC8 = "IFZC8";
const string gc_sCfgIfH1 = "IFH1";
const string gc_sCfgIfH2 = "IFH2";
const string gc_sCfgIfCmd = "IFCMD";
const string gc_sCfgNetMagModule = "net_mgr";
const string gc_sCfgSvAgent = "sv_agent";

//缺省命令字匹配
const string gc_sDefaultCmdID = "#";

class CGessTimerMgr;
class CConfigImpl;
class CNetMgrModule;
class CDeliverMgr;
class CIfSvAgent;
class CCvgCpMgr:public CProtocolCpMgr
{
private:
	//命令行处理线程
	class CCommandLineThread :public CWorkThreadNm
	{
	public:
		CCommandLineThread():m_pParent(0){}
		virtual ~CCommandLineThread(){}
		void Bind(CCvgCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			string sIn("");	
			cout << "Cvg->";
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

				CRLog(E_SYSINFO,"Cvg_CmdLine_Thread exit!");
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
		CCvgCpMgr* m_pParent;
	};

	//网络日志类消息处理线程
	class CNetLogThread :public CWorkThreadNm
	{
	public:
		CNetLogThread():m_pParent(0){}
		virtual ~CNetLogThread(){}
		void Bind(CCvgCpMgr* p){m_pParent = p;}
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

				CRLog(E_SYSINFO,"Offering_NetLog Thread exit!");
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
		CCvgCpMgr* m_pParent;
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
			m_pMgr=dynamic_cast<CCvgCpMgr*>(pCpMgr); 
			m_ulKey=ulKey;
		}
		int SendPacket(CPacket &GessPacket)
		{
			if (0==m_pMgr)
				return -1;
			return m_pMgr->OnPacketCmd(GessPacket);
		}
	private:
		CCvgCpMgr* m_pMgr;
		unsigned long m_ulKey;
	};

	//日志主机
	class CNetLogHost:public CSubscriber
	{
	public:
		CNetLogHost():m_pMgr(0){}
		virtual ~CNetLogHost(){}
		void Bind(CCvgCpMgr* p){m_pMgr = p;}
		void OnNotify(const string& sMsg)
		{
			if (0 != m_pMgr)
				m_pMgr->OnNetLogMsg(sMsg);
		}
	private:
		CCvgCpMgr* m_pMgr;
	};

	//心跳定时器
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CCvgCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CCvgCpMgr* m_pParent;
	};

	
	//同步时间定时器
	class CSyncTimer : public CGessTimer
	{
	public:
		CSyncTimer():m_pParent(0){}
		virtual ~CSyncTimer(){}
		void Bind(CCvgCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnSyncTime(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CCvgCpMgr* m_pParent;
	};

	//定时重启定时器
	class CResetTimer: public CGessAbsTimer
	{
	public:
		CResetTimer():m_pParent(0){}
		~CResetTimer(void){}
		void Bind(CCvgCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& sKey)
		{
			if (0 != m_pParent)
				return m_pParent->OnResetTimeout(sKey);
			return -1;
		}
		void TimerCanceled(const string& ulKey){}
	private:
		CCvgCpMgr* m_pParent;
	};
public:
	CCvgCpMgr();
	virtual ~CCvgCpMgr();
	
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
	void StopMe();
	int StartMe();

	int HandleConsolMsg(unsigned int uiMsg);

    // added by Jerry Lee, 2010-12-21, 历史数据保存路径
    string m_strHisDataPath;

    // added by Jerry Lee, 2011-1-17, tick数据保存路径
    string m_strTickDataPath;

    // added by Jerry Lee, 2011-2-14, 资讯索引数据保存路径 
    string m_strInfoIndexPath;

    // added by Jerry Lee, 2011-2-26, 资讯内容数据保存路径 
    string m_strContentPath;

	// added by Jimmy Jiao,2012-5-11, 存储方式的类型
	int m_cStoreType;

	// added by ZYB, 2012-6-11, 资讯数据库 ODBC连接
	string m_szInfoODBC;

    CInfoPublisher m_InfoPubr;

private:


	//定义成员函数指针
	typedef string (CCvgCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
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
	int OnSyncTime(const string& sTmKey,unsigned long& ulTmSpan); // added by zyb 同步时间
	

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
	CConnectPointAsyn*		m_pCpInterfaceZC2;		//采集器与接收器接口
	CConnectPointAsyn*		m_pCpInterfaceZC3;		//采集器与接收器接口
	CConnectPointAsyn*		m_pCpInterfaceZC4;		//采集器与接收器接口
	CConnectPointAsyn*		m_pCpInterfaceZC5;		//采集器与接收器接口
	CConnectPointAsyn*		m_pCpInterfaceZC6;		//采集器与接收器接口
	CConnectPointAsyn*		m_pCpInterfaceZC7;		//采集器与接收器接口
	CConnectPointAsyn*		m_pCpInterfaceZC8;		//采集器与接收器接口
	CConnectPointAsyn*		m_pServiceHandler1;		//
	CConnectPointAsyn*		m_pServiceHandler2;		//
	CConnectPointAsyn*		m_pServiceHandler3;		//
	CConnectPointAsyn*		m_pServiceHandler4;		//
	CConnectPointAsyn*		m_pServiceHandler5;		//
	CConnectPointAsyn*		m_pServiceHandler6;		//
	CConnectPointAsyn*		m_pServiceHandler7;		//
	CConnectPointAsyn*		m_pServiceHandler8;		//
	CConnectPointAsyn*		m_pHisDataHandler;		//
	CConnectPointAsyn*		m_pServiceHandlerSvr;	//
	CDeliverMgr*			m_pDeliverMgr;          //
	CNetMgrModule*			m_pNetMagModule;		//网管代理
	CConnectPointCmd		m_oCpCmdHandler;		//Telnet命令行处理
	CIfSvAgent*				m_pSvAgent;				//银行端监控代理

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


	//
	CCondMutex	m_deqCondMutex;

	//定时器管理器
	CGessTimerMgr* m_pGessTimerMgr;
	//K接口定时器
	CDogTimer m_oIfkTimer;

	//定时重启定时器
	CResetTimer m_oResetTimer;
	//定时重启时间点配置
	vector<CGessTime> m_vResetTime;	


	//同步时间定时器
	CSyncTimer m_oSyncTimer;


	//接口连接点连接数量
	int m_nConNumIf[EnumKeyUnknown];
	CGessMutex	m_csConNum;

	unsigned int		  m_uiNodeID;
	unsigned int		  m_uiNodeType;
	CConfigImpl*		m_pConfig;
	volatile bool m_bStop;

	// 是否接受时间同步
	bool m_bAcceptSyncTime;
	// 向下发起时间同步
	bool m_bSync_time;

private:
	//命令行命令分发
	string OnCmd(const string& sCmdLine, const vector<string>& vecPara);

	string OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineMem(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineHelp(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineEvtTest(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineSysInfo(const string& sCmdLine, const vector<string>& vecPara);
	string OnCmdLineQue(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara);

    // added by Jerry Lee, for sending history data, 2010-12-21
    string OnCmdLineSend(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineQuitNode(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineSetTime(const string& sCmd, const vector<string>& vecPara);
private:
	//Quit报文
	int OnRecvQuit(CIpcPacket& pkt);

	int CancelOrder();
	int HandleCmdSpecial(const string& sCmdID, CPacket &pkt);
};

#endif