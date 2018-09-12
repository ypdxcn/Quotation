#pragma once
#include "PacketBodyStructIpcOffer.h"
#include "OfferConstant.h"
#include "MibConstant.h"
#include "WorkThreadNm.h"
#include "Comm.h"
#include "Logger.h"
#include "IpcPacket.h"

using namespace MibConst;
using namespace OfferConst;
using namespace ipcoffer;

class CLoginMgr:public CConnectPointAsyn, public CWorkThreadNm
{
private:
	//登录定时器,用于登录失败后间歇性的重新尝试
	/*class COperationTimer : public CGessTimer
	{
	public:
		COperationTimer():m_pParent(0){}
		virtual ~COperationTimer(){}
		void Bind(CLoginMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			//if (0 != m_pParent)
			//	return m_pParent->LoginTimeout(ulKey,ulTmSpan);

			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CLoginMgr* m_pParent;
	};
*/
	//网管接口
	class CLoginNm: public CNmoModule
	{
	public:
		CLoginNm():m_pParent(0){}
		virtual ~CLoginNm(){}
		void Bind(CLoginMgr* pParent){m_pParent = pParent;}
		//单个查询接口
		int Query(CNMO& oNmo) const
		{
			if (0 == m_pParent)
				return -1;

			return m_pParent->LoginQuery(oNmo);
		}
		//批量查询接口
		int Query(vector< CNMO > & vNmo) const
		{
			if (0 == m_pParent)
				return -1;

			for (vector< CNMO >::iterator it = vNmo.begin(); it != vNmo.end(); ++it)
			{
				m_pParent->LoginQuery(*it);
			}
			return 0;
		}
		//控制接口
		int Operate(const string &sOid, int nflag, const string &sDstValue, const string &sParamer) 
		{
			if (0 == m_pParent)
				return -1;
			return m_pParent->LoginOperate(sOid, nflag, sDstValue, sParamer);
		}
	private:
		CLoginMgr* m_pParent;
	};
public:	
	CLoginMgr();
	virtual ~CLoginMgr();
public:
	//如下是继承自CConnectPointAsyn
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);	
	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int SendPacket(CPacket &GessPacket);
	int OnRecvPacket(CPacket &GessPacket);

	int LogoutInd();


	
	//登录登出结果通知,报盘模块调用
	void OnLogin(int nResult);
	void OnLogout(int nResult);
 
	//连接对端结果通知
	void ConnectNtf(int nFlag);
	//连接对端中断通知
	void DisconnectNtf();
	//接收到对端连接通知
	void AcceptNtf();

private:
	int ThreadEntry();
	int End();
	
	//主动类线程状态是否需要被网管
	bool IsNetManaged(string& sKeyName);
	//网管查询
	int LoginQuery(CNMO& oNmo) const;
	//网管控制接口
	int LoginOperate(const string &sOid, int nflag, const string &sDstValue, const string &sParamer) {return 0;}
	//是否告警状态
	int GetAlarmStat() const;

	//是否需要登录
	bool IsNeedLogin();
	//是否需要登出
	bool IsNeedLogout();
	//执行登录
	int PrepareLogin();
	//是否已经登录
	bool IsLogined();

	//发送状态信息
	int SendStateInfo(bool blMagicGen = false);
	//状态信息处理
	bool HandleStateInfo(StateInfo& stBody,int& nResult);
	
	//定时器超时处理函数
	int LoginTimeout(const string& ulKey,unsigned long& ulTmSpan);

	CConnectPointManager*	m_pCpMgr;		//连接点管理器
	unsigned int	m_ulKey;				//连接点key
	CConfig *		m_pCfg;					//配置接口
	std::deque<CIpcPacket> m_deqMIf;		//双机状态信息报文队列
	CCondMutex	m_deqCondMutex;

	typedef struct tagLoginStateInfo
	{
		int	nMasterSlave;					//主从标志配置1主 0从
		int nLoginIndication;				//登录指示 0指示登录 1指示登出
		int nLoginToken;					//登录令牌 0持有 1未持有
		time_t			tmToken;			//获取令牌的时间戳		
		int nLoginState;                    //登录状态
		int nLoginResult;                   //登录结果
		int nMagicNum;						//随机数
		bool blInfoSended;					//已发送标志

		tagLoginStateInfo()
		{
			nMasterSlave = gc_nMaster;
			nLoginIndication = gc_nIndUnknown;
			nLoginToken = gc_nLoginTokenWait;
			tmToken = 0;
			nLoginState = gc_nStateLoginInit;
			nLoginResult = gc_nStateLoginInit;
			nMagicNum = 0;
			blInfoSended = false;
		}
	} LOGIN_STATE_INFO, *PLOGIN_STATE_INFO;

	LOGIN_STATE_INFO m_stLoginInfoLocal;	//本端登录信息
	LOGIN_STATE_INFO m_stLoginInfoRemote;	//对端登录信息
	bool m_blTokenConflict;					//令牌冲突
	int	m_nConnectState;					//与对端连接状态
	bool m_blFirstConnectState;				//是否第一次启动后的第一次连接状态信息
	int	m_nNodeID;							//节点ID
	mutable CGessMutex m_csState;

	CLoginNm			 m_oNm;				//网管接口模块
private:
	//定义成员函数指针
	typedef int (CLoginMgr::*MFP_PacketHandleApi)(CIpcPacket& pkt);

	//报文命令字与报文处理成员函数映射结构
	typedef struct tagCmd2Api
	{
		string sApiName;						//报文ApiName或交易代码
		MFP_PacketHandleApi pMemberFunc;		//报文处理函数指针
	} Cmd2Api;

	//报文命令字与报文处理成员函数映射表
	static Cmd2Api m_Cmd2Api[];

	int RunPacketHandleApi(CIpcPacket& pkt);
private:
	int OnStateInfo(CIpcPacket& pkt);
	int OnStateInfoRsp(CIpcPacket& pkt);
};
