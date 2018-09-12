//协议流程 适用于 
#ifndef _PROCESS_INTERFACE_YC_H
#define _PROCESS_INTERFACE_YC_H

#include "CommApSampler.h"
#include "ProcessInterfaceNm.h"
#include "BroadcastPacket.h"

using namespace std;

class COMMAPSAMPLER_CLASS CProcessInterfaceYC : public CProcessInterfaceClnNm
{
public:
	CProcessInterfaceYC(void);
	~CProcessInterfaceYC(void);

	typedef struct tagGessPktInfo
	{
		unsigned long ulIdleInterval;		//空闲时正常发送Hello间隔时间，也是链路上的最长净空时间
		unsigned long ulIntervalReSend;		//无应答后重检查间隔
		unsigned long ulHelloReSend;		//无应答而重发心跳的次数
		bool blNeedLogin;					//是否需要登录

		std::string	node_type;
		std::string	node_id;
		std::string	node_name;
		std::string	host_id;
		std::string	sUserName;				//做客户时向服务端登录用户名
		std::string	sPassword;				//做客户时向服务端登录密码

		std::string	term_type;	//
		std::string	user_key;
		std::string	user_type;
		std::string	user_id;
		std::string	user_pwd;
		std::string	branch_id;
		std::string	again_flag;
	} GessPktInfo,*PGessPktInfo;

	int Init(CConfig * pCfg);

	virtual int OnConnect();		// 作客户端连接成功后回调
	virtual void OnClose();	        // 断开连接时调用
	int GetNmKey(string& sKey);
protected:
	//父类定义的回调函数实现
	virtual int OnPacket(char * pData, size_t nSize);
	virtual void GetPacketInfo(PacketInfo & stInfo);//报文格式信息
	int HandleTimeout(unsigned long& ulTmSpan);

	virtual bool IsLogin() {return m_blIsLogin;} //登录状态

	//协议定义的交互报文
	virtual int SendHello();
	virtual int OnHello(CBroadcastPacket & GessPacket);
	virtual int SendLogin();
    virtual int OnRspLogin(CBroadcastPacket & GessPacket);
private:
	GessPktInfo m_GessPktInfo;
	bool		m_blIsLogin;	//登录状态
	
	CConfig *	m_pCfg;
	//std::deque<CBroadcastPacket> m_qSend;
	CGessMutex m_csYC;

	unsigned long m_uiCountNoAlive;			//超时无来报文次数
	unsigned long m_uiCountSended;			//无应答后已发送心跳次数
	unsigned long m_uiCountNoQuote;			//超时无来实时报价次数  added by Ben 20110429
	bool m_bGessMarketActive;               //是否处于交易时间段的标志 added by Ben 20110429
	bool m_bGessInstActive;               
};
#endif