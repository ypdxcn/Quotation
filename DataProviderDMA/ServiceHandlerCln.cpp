#include "ServiceHandlerCln.h"
#include "ProviderCpMgr.h"

//报文对应的处理成员函数配置表
CServiceHandlerCln::PktCmd2Api CServiceHandlerCln::m_PktCmd2Api[] =
{
	//请求ApiName					请求处理函数指针
	{"80000001",							&CServiceHandlerCln::OnRecvLoginRsp},
	{"80000002",							&CServiceHandlerCln::OnRecvLogoutRsp},
	{"80000003",							&CServiceHandlerCln::OnRecvSubscripRsp},
	{"80000004",							&CServiceHandlerCln::OnRecvUnSubscripRsp},
	{"80000005",							0},//&CServiceHandlerCln::OnRecvHelloRsp}
};

CServiceHandlerCln::CServiceHandlerCln(void)
:m_pProviderCpMgr(0)
,m_pCfg(0)
{
}

CServiceHandlerCln::~CServiceHandlerCln(void)
{
}

//初始化配置，创建线程池
int CServiceHandlerCln::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	//CConfig* pConfig = m_pCfg->GetCfgGlobal();

	return 0;
}

//启动调度线程及工作线程池
int CServiceHandlerCln::Start()
{
	//启动调度线程
	BeginThread();
	return 0;
}

//停止工作线程池及调度线程
void CServiceHandlerCln::Stop()
{
	//停止调度线程
	CRLog(E_APPINFO,"%s","Stop ServiceHandler Thread");
	EndThread();
}

//清理资源
void CServiceHandlerCln::Finish()
{
	m_deqService.clear();
}

void CServiceHandlerCln::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pProviderCpMgr = dynamic_cast<CProviderCpMgr*>(pCpMgr);
}


int CServiceHandlerCln::SendPacket(CPacket &pkt)
{
	try
	{
		CSamplerPacket & pktService = dynamic_cast<CSamplerPacket&>(pkt);
				
		std::string sCmdID = pkt.GetCmdID();
		if (sCmdID == "00000006")
		{
			CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pktService.GetMsg());
			string sQuotationVal;
			msg.GetBinaryField(MSG_QUOTATION_RECS,sQuotationVal);
			
			if (0 != m_pProviderCpMgr)
			{
				m_pProviderCpMgr->ToHisDataFx(sQuotationVal);
			}
			return 0;
		}

		m_deqCondMutex.Lock();
		m_deqService.push_back(pktService);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		return 0;
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"%s","packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

int CServiceHandlerCln::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqService.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			CSamplerPacket pkt = m_deqService.front();
			m_deqService.pop_front();
			m_deqCondMutex.Unlock();

			try
			{
				RunPacketHandleApi(pkt);
			}
			catch(...)
			{
				CRLog(E_CRITICAL,"%s","Unknown exception");
			}
		}
		CRLog(E_APPINFO,"%s","RiskHandler Thread exit!");
		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

int CServiceHandlerCln::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}

//匹配报文处理成员函数并进行调用处理 
int CServiceHandlerCln::RunPacketHandleApi(CSamplerPacket& pkt)
{
	std::string sCmdID;
	try
	{
		sCmdID = pkt.GetCmdID();
	
		int nSize = sizeof(m_PktCmd2Api)/sizeof(PktCmd2Api);
		for ( int i = 0 ; i < nSize ; i++ )
		{
			if ( m_PktCmd2Api[i].sApiName == sCmdID )
			{
				if (m_PktCmd2Api[i].pMemberFunc == 0)
					break;

				return (this->*(m_PktCmd2Api[i].pMemberFunc))(pkt);				
			}
		}

		CRLog(E_ERROR,"Unknown packet! sCmdID= %s" ,sCmdID.c_str());
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s,Handle Packet:%s", e.what(),sCmdID.c_str());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception,Handle Packet:%s",sCmdID.c_str());
		return -1;
	}
}

int CServiceHandlerCln::OnRecvLoginRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);
	m_sNodeID = ToHexString<unsigned int>(uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_LOGIN_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvLoginRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);

	return 0;
}

int CServiceHandlerCln::OnRecvLogoutRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);
	m_sNodeID = ToHexString<unsigned int>(uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_LOGIN_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvLogoutRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);

	return 0;
}

int CServiceHandlerCln::OnRecvSubscripRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_SUBSCRIP_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvSubscripRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);


	return 0;
}

int CServiceHandlerCln::OnRecvUnSubscripRsp(CSamplerPacket& pkt)
{
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

	unsigned int uiSeqNo = 0;
	msg.GetField(MSG_SEQ_ID,uiSeqNo);

	unsigned int uiNodeID = 0;
	msg.GetField(MSG_NODE_ID,uiNodeID);

	unsigned int uiRst = 0;
	msg.GetField(MSG_SUBSCRIP_RESULT,uiRst);
	CRLog(E_PROINFO,"OnRecvUnSubscripRsp SeqNo:%u, NodeID:%u, Result:%u",uiSeqNo, uiNodeID, uiRst);


	return 0;
}


//int CServiceHandlerCln::OnRecvHelloRsp(CSamplerPacket& pkt)
//{
//	
//	CRLog(E_APPINFO,"OnRecvHelloRsp");
//	return 0;
//}
