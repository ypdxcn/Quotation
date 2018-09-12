#ifndef _SERVICE_HANDLER_H
#define _SERVICE_HANDLER_H
#include "workthread.h"
#include "SamplerPacket.h"

class CProviderCpMgr;
class CServiceHandlerSvr :public CConnectPointAsyn, public CWorkThread
{
public:
	CServiceHandlerSvr(void);
	~CServiceHandlerSvr(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);

private:
	//定义成员函数指针
	typedef int (CServiceHandlerSvr::*MFP_PktHandleApi)(CSamplerPacket& pkt);
	//报文命令字与报文处理成员函数映射结构
	typedef struct tagPktCmd2Api
	{
		string sApiName;						//报文ApiName或交易代码
		MFP_PktHandleApi pMemberFunc;			//报文处理函数指针
	} PktCmd2Api;
	//报文命令字与报文处理成员函数映射表
	static PktCmd2Api m_PktCmd2Api[];
	int RunPacketHandleApi(CSamplerPacket& pkt);

	CProviderCpMgr*     m_pProviderCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;

	std::deque<CSamplerPacket> m_deqService;
	CCondMutex	m_deqCondMutex;

	int ThreadEntry();
	int End();

	int OnRecvLogin(CSamplerPacket& pkt);
	int OnRecvSubscrip(CSamplerPacket& pkt);
	int OnRecvLogout(CSamplerPacket& pkt);
	int OnRecvUnSubscrip(CSamplerPacket& pkt);
	int OnRecvHello(CSamplerPacket& pkt);

};
#endif