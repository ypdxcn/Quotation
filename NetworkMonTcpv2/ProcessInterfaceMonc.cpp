#include "ProcessInterfaceMonc.h"
#include "Logger.h"
#include "strutils.h"

using namespace strutils;

CProcessInterfaceMonc::CProcessInterfaceMonc()
{
	m_GessPktInfo.ulIdleInterval = 5;//DEFAULT_IDLE_TIMEOUT;
	m_GessPktInfo.ulIntervalReSend = DEFAULT_HELLO_RESEND_INTERVAL;
	m_GessPktInfo.ulHelloReSend = DEFAULT_HELLO_RESEND_COUNT;
	m_GessPktInfo.blNeedLogin = false;
	m_GessPktInfo.nNeedCheckServicePkt = 0;
}

CProcessInterfaceMonc::~CProcessInterfaceMonc(void)
{
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
void CProcessInterfaceMonc::GetPacketInfo(PacketInfo & stInfo)
{
	stInfo.eLengthType = ltInterger;
	stInfo.nLengthPos = 0;
	stInfo.nLengthBytes = 4;
	stInfo.blLenIncludeHeader = true;
	stInfo.nFixHeadLen = 12;
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
int CProcessInterfaceMonc::OnPacket(char * pData, size_t nSize)
{
	CBinBlockPkt oPacket;
	oPacket.Decode(pData, nSize);

	OnRecvPacket(oPacket);
	return 0;
}

/******************************************************************************
函数描述:socket连接成功后，作为客户端被通知,一般用于触发认证报文发送
调用函数:通讯处理器对象回调
返回值  :int 暂无特殊处理
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
int CProcessInterfaceMonc::OnConnect()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MONC OnConnect socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);
	return 0;
}


/******************************************************************************
函数描述:socket连接中断后则被通知,可根据协议要求进行处理
调用函数:父类对象回调
返回值  :int 暂无特殊处理
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
void CProcessInterfaceMonc::OnClose()
{
	string sLocalIp,sPeerIp;
	int nLocalPort,nPeerPort;
	GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
	CRLog(E_PROINFO,"MONC OnClose socket (%d) Peer(%s:%d),Me(%s:%d)",SocketID(),sPeerIp.c_str(),nPeerPort,sLocalIp.c_str(),nLocalPort);

	return;
}