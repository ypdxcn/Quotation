#ifndef _NETWORK_MONITOR_CP_MGR_H
#define _NETWORK_MONITOR_CP_MGR_H
#include "Comm.h"
#include <vector>
#include <string>
#include <iostream>
#include "NmService.h"
//#include "MemShareAlive.h"

using namespace std;



//连接点key常量
typedef enum tagEnumKey
{
	EnumKeyNmSvr,
	EnumKeyIfMons,
	EnumKeyIfMonc1,
	EnumKeyIfMonc2,
	EnumKeyIfMonc3,
	//
	EnumKeyReserve,
	EnumKeyUnknown
} EnumKeyIf;

//各个连接点配置项名
const string gc_sCfgNmService = "NMSERVICE";
const string gc_sCfgIfMons = "IFMONS";

//缺省命令字匹配
const string gc_sDefaultCmdID = "#";

class CGessTimerMgr;
class CConfigImpl;
class CNmCpMgr:public CProtocolCpMgr
{
private:
	//命令行处理线程
	class CCommandLineThread :public CWorkThread
	{
	public:
		CCommandLineThread():m_pParent(0){}
		virtual ~CCommandLineThread(){}
		void Bind(CNmCpMgr* p){m_pParent = p;}
	private:
		int ThreadEntry()
		{
			string sIn("");	
			cout << "NmMon->";
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

				CRLog(E_SYSINFO,"NmMon_CmdLine_Thread exit!");
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
	private:
		CNmCpMgr* m_pParent;
	};

	//心跳定时器
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CNmCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CNmCpMgr* m_pParent;
	};

public:
	CNmCpMgr();
	virtual ~CNmCpMgr();
	
	int OnConnect(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	int OnAccept(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);	
	int OnClose(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);
	int OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	//int OnLogout
	int Forward(CPacket &GessPacket,const unsigned long& ulKey);

	int Init(const string& sProcName = "NmMon");
	void Finish();
	int Start();
	void Stop();
	int Run();

	int ToMoncx(CPacket &oPacket,const unsigned long& ulKey);
	int ToMons(CPacket &oPacket);
private:	
	//定义成员函数指针
	typedef string (CNmCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
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

	
	int HandleCmdLine(string& sIn);
	int OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan);

	//命令行命令分发
	string OnCmd(const string& sCmdLine, const vector<string>& vecPara);
	string OnCmdLineQuit(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineHelp(const string& sCmd, const vector<string>& vecPara);
	
private:
	std::string				m_sProcName;			//进程名称
	
	CConnectPointAsyn*		m_pCpInterfaceMons;		//
	
	CConfigImpl*			m_pCfgMonc1;
	CConnectPointAsyn*		m_pCpInterfaceMonc1;		//
	
	CConfigImpl*			m_pCfgMonc2;
	CConnectPointAsyn*		m_pCpInterfaceMonc2;		//

	CConfigImpl*			m_pCfgMonc3;
	CConnectPointAsyn*		m_pCpInterfaceMonc3;		//
	CNmService*				m_pNmService;

	//看门狗共享内存
	//CMemShareAlive			m_oMemShareAlive;

	//命令行处理线程
	CCommandLineThread		m_oCmdLineThread;

	
	//定时器管理器
	//CGessTimerMgr* m_pGessTimerMgr;
	//看门狗定时器
	//CDogTimer m_oIfkTimer;


	//主线程控制条件变量
	CCondMutex				m_deqCondMutex;

	CConfigImpl*			m_pConfig;
	volatile bool			m_bStop;
};
#endif