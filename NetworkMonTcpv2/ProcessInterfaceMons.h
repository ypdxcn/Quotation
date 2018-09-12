//协议流程 适用于如下接口的服务端流程:
//
#ifndef _PROCESS_INTERFACE_MONS_H
#define _PROCESS_INTERFACE_MONS_H


#include "PktBinBlockMon.h"
#include "ProtocolProcess.h"


#define DEFAULT_IDLE_TIMEOUT						16
#define DEFAULT_HELLO_RESEND_INTERVAL				6
#define DEFAULT_HELLO_RESEND_COUNT					8


class CProcessInterfaceMons : public CTcpAppProcessServer
{
public:
	CProcessInterfaceMons(void);
	~CProcessInterfaceMons(void);

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
	} GessPktInfo,*PGessPktInfo;

	int Init(CConfig * pCfg);

	virtual int OnAccept();					// 作服务端接收到连接后回调
	virtual void OnClose();					// 断开连接时调用
protected:
	//父类定义的回调函数实现
	virtual int OnPacket(char * pData, size_t nSize);
	virtual void GetPacketInfo(PacketInfo & stInfo);//报文格式信息
	virtual int HandleTimeout(unsigned long& ulTmSpan) {return 0;}

private:
	static GessPktInfo m_GessPktInfo;
	static bool	m_blGessPktInfoInited;
	
	CConfig *	m_pCfg;
	CGessMutex m_csZS;
};
#endif
