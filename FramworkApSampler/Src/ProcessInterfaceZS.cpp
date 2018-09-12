#include "ProcessInterfaceZS.h"
#include "Logger.h"
#include "strutils.h"

CProcessInterfaceZS::GessPktInfo CProcessInterfaceZS::m_GessPktInfo = 
{
	DEFAULT_IDLE_TIMEOUT,
	DEFAULT_HELLO_RESEND_INTERVAL,
	DEFAULT_HELLO_RESEND_COUNT,
	false
};
bool CProcessInterfaceZS::m_blGessPktInfoInited = false;

CProcessInterfaceZS::CProcessInterfaceZS()
:m_pCfg(0)
,m_uiCountNoAlive(0)
,m_uiCountSended(0)
{
}

CProcessInterfaceZS::~CProcessInterfaceZS(void)
{
}

int CProcessInterfaceZS::Init(CConfig * pCfg)
{
	CTcpAppProcessServer::Init(pCfg);
	if (!m_blGessPktInfoInited)
	{
		m_blGessPktInfoInited = true;
		
		string sLoginFlag;
		if (0 == pCfg->GetProperty("login",sLoginFlag))
		{
			if (1 == FromString<int>(sLoginFlag))
				m_GessPktInfo.blNeedLogin = true;
		}

		CConfig* pCfgGlobal = pCfg->GetCfgGlobal();
		pCfgGlobal->GetProperty("node_type",m_GessPktInfo.node_type);
		pCfgGlobal->GetProperty("node_id",m_GessPktInfo.node_id);
		pCfgGlobal->GetProperty("host_id",m_GessPktInfo.node_name);
		pCfgGlobal->GetProperty("host_id",m_GessPktInfo.host_id);	
	}

	m_pCfg = pCfg;
	return 0;
}

/******************************************************************************
函数描述:获取协议定义的报文格式信息
调用函数:父类对象回调
输出参数:PacketInfo & stInfo 报文格式信息
        eLengthType 长度字段类型 整型/10进制字符串/16进制字符串
		nLengthBytes 消息包长度字段的长度,为整数时只支持 1 2 4字节
		nLengthPos 长度字段起始位置
		blLenIncludeHeader 长度是否含报文头
		nFixHeadLen  除长度字段指示的长度外,报文固定部分长度
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
void CProcessInterfaceZS::GetPacketInfo(PacketInfo & stInfo)
{
	stInfo.eLengthType = ltInterger;
	stInfo.nLengthPos = 0;
	stInfo.nLengthBytes = 4;
	stInfo.blLenIncludeHeader = true;
	stInfo.nFixHeadLen = 16;
}

/******************************************************************************
函数描述:协议报文处理最主要函数,根据报文命令字进行直接处理或转发
调用函数:父类对象回调
被调函数:OnRecvPacket,上传报文
输入参数:char * pData接收缓存, size_t nSize缓存大小
返回值  :int 暂无特殊处理
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
int CProcessInterfaceZS::OnPacket(char * pData, size_t nSize)
{	
	CSamplerPacket oPacket;
	oPacket.Decode(pData, nSize);
	
	bool blFlag = false;
	m_csZS.Lock();
	if (m_uiCountSended > 1)
		blFlag = true;
	m_uiCountNoAlive = 0;
	m_csZS.Unlock();

	std::string sCmdID = oPacket.GetCmdID();
	if (strutils::ToHexString<unsigned int>(YL_HELLO_RSP) == sCmdID)
	{
		if (blFlag)
			CRLog(E_PROINFO,"ZS收到心跳应答(%u)",m_uiCountSended);		
		return 0;
	}
	else if (strutils::ToHexString<unsigned int>(YL_HELLO) == sCmdID)
	{
		return OnHello(oPacket);
	}
	else
	{
		//if (!IsLogin())
		//	return -1;

		// 
		std::string sKey = oPacket.RouteKey();
		OnRecvPacket(sKey,oPacket);
	}

	return 0;
}


// 作服务端接收到连接后回调
int CProcessInterfaceZS::OnAccept()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"ZS OnAccept socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);
	CreateTimer(m_GessPktInfo.ulIdleInterval);
	CProcessInterfaceSvrNm::OnAccept();
	return 0;
}

/******************************************************************************
函数描述:socket连接中断后则被通知,可根据协议要求进行处理
调用函数:通讯处理器对象回调
返回值  :int 暂无特殊处理
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
void CProcessInterfaceZS::OnClose()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"ZS Long Connection Close socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);

	CProcessInterfaceSvrNm::OnClose();
	DestroyTimer();
	return;
}


//处理定时器超时
int CProcessInterfaceZS::HandleTimeout(unsigned long& ulTmSpan)
{
	int nRtn = 0;
	CMutexGuard oGuard(m_csZS);
	if (m_uiCountNoAlive >= 1)
	{//超过链路最大空闲时间未收到报文，则发送心跳
		if (m_uiCountSended >= m_GessPktInfo.ulHelloReSend)
		{//重发心跳次数超过规定次数则准备关闭
			nRtn = -1;
			ReqClose();

			CRLog(E_PROINFO,"ZS%s(%u-%u)","心跳超时!",m_uiCountNoAlive,m_uiCountSended);
		}
		else
		{//重置定时器间隔
			ulTmSpan = m_GessPktInfo.ulHelloReSend;
			m_uiCountSended++;

			if (m_uiCountSended > 1)
				CRLog(E_PROINFO,"ZS发送心跳,第%d次!",m_uiCountSended);

			int nSended = SendHello();
			if (nSended < 0 && nSended != -2)
				nRtn = -1;
		}
	}
	else
	{
		m_uiCountSended = 0;
		ulTmSpan = m_GessPktInfo.ulIdleInterval;
	}
	m_uiCountNoAlive++;
	return nRtn;
}

//发送心跳
int CProcessInterfaceZS::SendHello()
{
	CSamplerPacket oPacketHello(YL_HELLO);
	CMessage &  msg = oPacketHello.GetMsg();
	msg.SetField(MSG_SEQ_ID,static_cast<unsigned int>(0));
	msg.SetField(MSG_NODE_ID,static_cast<unsigned int>(0));
	
	return CAppProcess::SendPacket(oPacketHello);
}

//处理心态检测报文
int CProcessInterfaceZS::OnHello(CSamplerPacket & oPacket)
{
    CMessage &  msgReq = oPacket.GetMsg();
	unsigned int uiSeqID = 0;
	msgReq.GetField(MSG_SEQ_ID,uiSeqID);
	unsigned int uiNodeID = 0;
	msgReq.GetField(MSG_NODE_ID,uiNodeID);

	CSamplerPacket oPacketRsp(YL_HELLO_RSP);
	CMessage &  msgRsp = oPacket.GetMsg();
	msgRsp.SetField(MSG_SEQ_ID,uiSeqID);
	msgRsp.SetField(MSG_NODE_ID,uiNodeID);

	CAppProcess::SendPacket(oPacketRsp);
	return 0; 
}

int CProcessInterfaceZS::GetNmKey(string& sKey)
{
	sKey = "ZS接口服务端连接.";
	int nSockeID = static_cast<int>( SocketID());
	if (INVALID_SOCKET != nSockeID)
	{
		sKey += ToString<int>(nSockeID);
	}
	else
	{
		srand(static_cast<unsigned int>(time(0)));
		int RANGE_MIN = 1;
		int RANGE_MAX = 99;
		int nRandom = rand() * (RANGE_MAX - RANGE_MIN) / RAND_MAX + RANGE_MIN;
		sKey += ToString<int>(nRandom);
	}

	return 0;
}