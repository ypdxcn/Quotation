
#ifndef _INTERFACE_AGENT_SV_H
#define _INTERFACE_AGENT_SV_H

#include "SvAgent.h"
#include "workthread.h"
#include "SamplerPacket.h"
#include "BroadcastPacket.h"
#include "TradePacket.h"

using namespace std;

class CNetMgrModule;
class SVAGENT_CLASS CIfSvAgent : public CConnectPointAsyn, public CWorkThread
{
public:
	CIfSvAgent(void);
	~CIfSvAgent(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);

	void SetObj(CNetMgrModule* pNetMgrModule, CConnectPointAsyn* pIfH1,  CConnectPointAsyn* pIfH2);
private:
	CNetMgrModule* m_pNetMgrModule;
	CConnectPointAsyn* m_pIfH1;
	CConnectPointAsyn* m_pIfH2;
	CConnectPointManager*     m_pCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;

	//本机节点号
	unsigned int	m_uiNodeID;

	//路由信息结构
	typedef struct
	{
		//紧邻下级路由节点
		unsigned int uiNextNode;
		//节点串
		string sNodes;
	} NODE_LIST;
	//每个下级节点的路由信息
	map<unsigned int, NODE_LIST>		m_mapNodeRoute;

	//CSamplerPacket订阅转发类
	std::deque<CSamplerPacket> m_deqSv;
	//H2广播报文处理 主要是事件告警等
	std::deque<CBroadcastPacket> m_deqH2;
	//H1交易类报文处理 监控项查询
	std::deque<CTradePacket> m_deqH1;
	CCondMutex	m_deqCondMutex;

	int HandleSamplerPacket(CSamplerPacket& oPkt);
	int HandleH1Packet(CTradePacket& oPkt);
	int HandleH2Packet(CBroadcastPacket& oPkt);

	int ThreadEntry();
	int End();
};
#endif