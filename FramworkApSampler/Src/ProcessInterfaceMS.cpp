#include "ProcessInterfaceMS.h"
#include "IpcPacket.h"
#include "Logger.h"
#include "ApConstant.h"
using namespace ApConst;

CProcessInterfaceMS::GessPktInfo CProcessInterfaceMS::m_GessPktInfo = 
{
	8,//DEFAULT_IDLE_TIMEOUT,
	4,//DEFAULT_HELLO_RESEND_INTERVAL,
	8,//DEFAULT_HELLO_RESEND_COUNT,
	false
};
bool CProcessInterfaceMS::m_blGessPktInfoInited = false;

CProcessInterfaceMS::CProcessInterfaceMS()
:m_pCfg(0)
,m_uiCountNoAlive(0)
,m_uiCountSended(0)
{
}

CProcessInterfaceMS::~CProcessInterfaceMS(void)
{
}

int CProcessInterfaceMS::Init(CConfig * pCfg)
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

		string sVal;
		if (0 == pCfg->GetProperty("max_idle",sVal))
		{
			m_GessPktInfo.ulIdleInterval = strutils::FromString<unsigned long>(sVal);
			if (m_GessPktInfo.ulIdleInterval > 300 || m_GessPktInfo.ulIdleInterval < 2)
				m_GessPktInfo.ulIdleInterval = DEFAULT_IDLE_TIMEOUT;
		}

		if (0 == pCfg->GetProperty("resend_interval",sVal))
		{
			m_GessPktInfo.ulIntervalReSend = strutils::FromString<unsigned long>(sVal);
			if (m_GessPktInfo.ulIntervalReSend > 60 || m_GessPktInfo.ulIntervalReSend < 2)
				m_GessPktInfo.ulIntervalReSend = DEFAULT_HELLO_RESEND_INTERVAL;
		}

		if (0 == pCfg->GetProperty("resend_count",sVal))
		{
			m_GessPktInfo.ulHelloReSend = strutils::FromString<unsigned long>(sVal);
			if (m_GessPktInfo.ulHelloReSend > 30 || m_GessPktInfo.ulHelloReSend < 1)
				m_GessPktInfo.ulHelloReSend = DEFAULT_HELLO_RESEND_COUNT;
		}
		CRLog(E_DEBUG, "MS:max_idle:%u, resend_interval:%u, resend_count:%u",m_GessPktInfo.ulIdleInterval, m_GessPktInfo.ulIntervalReSend, m_GessPktInfo.ulHelloReSend);


		CConfig* pCfgGlobal = pCfg->GetCfgGlobal();
		pCfgGlobal->GetProperty("node_type",m_GessPktInfo.node_type);
		pCfgGlobal->GetProperty("node_id",m_GessPktInfo.node_id);
		pCfgGlobal->GetProperty("host_id",m_GessPktInfo.node_name);
		pCfgGlobal->GetProperty("host_id",m_GessPktInfo.host_id);	
	}

	m_pCfg = pCfg;	
	m_uiCountNoAlive = 0;
	m_uiCountSended = 0;
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
void CProcessInterfaceMS::GetPacketInfo(PacketInfo & stInfo)
{
	stInfo.eLengthType = ltCharactersDec;
	stInfo.nLengthPos = 0;
	stInfo.nLengthBytes = IPC_LENGTH_BYTES;
	stInfo.blLenIncludeHeader = false;
	stInfo.nFixHeadLen = stInfo.nLengthPos + stInfo.nLengthBytes;
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
int CProcessInterfaceMS::OnPacket(char * pData, size_t nSize)
{
	CIpcPacket GessPacket;
	GessPacket.Decode(pData, nSize);

	bool blFlag = false;
	m_csMS.Lock();
	if (m_uiCountSended > 1)
		blFlag = true;
	m_uiCountNoAlive = 0;
	m_csMS.Unlock();

	std::string sCmdID = GessPacket.GetCmdID();
	if (sCmdID == "Hello")
	{
		//CRLog(E_PROINFO,"MS收到心跳:%s",GessPacket.Print().c_str());
		return OnHello(GessPacket);
	}
	else if (sCmdID == "HelloRsp")
	{
		if (blFlag)
			CRLog(E_PROINFO,"MS收到心跳应答:%s",GessPacket.Print().c_str());
		return 0;
	}
	
	CRLog(E_PROINFO,"MS Recv:%s",GessPacket.Print().c_str());

	std::string sKey = GessPacket.RouteKey();
	OnRecvPacket(sKey,GessPacket);
	return 0;
}

// 作服务端接收到连接后回调
int CProcessInterfaceMS::OnAccept()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MS OnAccept and CreateTimer socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);
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
void CProcessInterfaceMS::OnClose()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MS Long Connection Close and DestroyTimer socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);

	DestroyTimer();
	CProcessInterfaceSvrNm::OnClose();
	return;
}

//处理定时器超时
int CProcessInterfaceMS::HandleTimeout(unsigned long& ulTmSpan)
{
	int nRtn = 0;
	m_csMS.Lock();
	if (m_uiCountNoAlive >= 1)
	{//超过链路最大空闲时间未收到报文，则发送心跳
		if (m_uiCountSended >= m_GessPktInfo.ulHelloReSend)
		{//重发心跳次数超过规定次数则准备关闭
			CRLog(E_PROINFO,"%s(%u-%u)","MS心跳超时!",m_uiCountNoAlive,m_uiCountSended);
			ReqClose();
			nRtn = -1;
		}
		else
		{//重置定时器间隔
			ulTmSpan = m_GessPktInfo.ulIntervalReSend;
			m_uiCountSended++;

			if (m_uiCountSended > 1)
				CRLog(E_PROINFO,"MS发送心跳,第%d次!",m_uiCountSended);

			//发送心跳
			int nSend = SendHello();
			if (nSend < 0)
			{
				nRtn = -1;
			}
		}
	}
	else
	{
		m_uiCountSended = 0;
		ulTmSpan = m_GessPktInfo.ulIdleInterval;
	}
	m_uiCountNoAlive++;
	m_csMS.Unlock();	
	return nRtn;
}

//发送心跳
int CProcessInterfaceMS::SendHello()
{
	CIpcPacket pkt("Hello");
	CAppProcess::SendPacket(pkt);
	return 0;
}

//发送心跳应答
int CProcessInterfaceMS::OnHello(CIpcPacket& pktHello)
{
	CIpcPacket pkt("HelloRsp");
	CAppProcess::SendPacket(pkt);
	return 0;
}


int CProcessInterfaceMS::GetNmKey(string& sKey)
{
	sKey = "M接口服务端连接.";
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