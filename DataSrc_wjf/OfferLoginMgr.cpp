#include "stdafx.h"
#include "Logger.h"
#include "OfferLoginMgr.h"
#include "PacketStructTransferIpcOffer.h"
#include "GessTimerMgrPosix.h"

#include "DataSrcCpMgr.h"

using namespace ipcoffer;

CLoginMgr::Cmd2Api CLoginMgr::m_Cmd2Api[] =
{
	{"StateInfo",	&CLoginMgr::OnStateInfo},
	{"StateRsp",	&CLoginMgr::OnStateInfoRsp}
};

CLoginMgr::CLoginMgr()
:m_pCpMgr(0)
,m_ulKey(0xFFFFFFFF)
,m_pCfg(0)
,m_blTokenConflict(false)
,m_nConnectState(gc_nStateInit)
,m_blFirstConnectState(true)
,m_nNodeID(0)
{
	m_stLoginInfoRemote.nLoginState = gc_nStateLoginUnknown;
}

CLoginMgr::~CLoginMgr()
{}

//匹配报文处理成员函数并进行调用处理 可以优化为哈希表查找
int CLoginMgr::RunPacketHandleApi(CIpcPacket& pkt)
{
	std::string sCmdID;
	try
	{
		sCmdID = pkt.GetCmdID();
	
		int nSize = sizeof(m_Cmd2Api)/sizeof(Cmd2Api);
		for ( int i = 0 ; i < nSize ; i++ )
		{
			if ( m_Cmd2Api[i].sApiName == sCmdID )
			{
				if (m_Cmd2Api[i].pMemberFunc == 0)
					break;

				return (this->*(m_Cmd2Api[i].pMemberFunc))(pkt);
			}
		}
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

void CLoginMgr::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	//连接点管理器
	m_pCpMgr = pCpMgr; 
	if (m_pCpMgr == 0)
	{
		CRLog(E_ERROR,"传递指针有误.");
	}

	//唯一标识连接点的key
	m_ulKey=ulKey;
}

int CLoginMgr::Init(CConfig* pConfig) 
{
	assert(0 != pConfig);
	if (0 == pConfig)
		return -1;

	m_pCfg = pConfig;	
	
	//网管接口
	m_oNm.Bind(this);

	//读取配置...
	string sCfgTmp = "1000";
	CConfig* pCfgGlobal = pConfig->GetCfgGlobal();
	if (0 == pCfgGlobal->GetProperty("node_id",sCfgTmp))
	{
		m_nNodeID = FromString<int>(sCfgTmp);
	}

	//主从配置
	sCfgTmp = ToString<int>(gc_nSlave);
	if (0 == pCfgGlobal->GetProperty("master",sCfgTmp))
	{
		int nFlag = FromString<int>(sCfgTmp);
		if (gc_nMaster == nFlag)
		{
			m_stLoginInfoLocal.nMasterSlave = gc_nMaster;
		}
		else
		{
			m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
		}
	}

	//是否初始化令牌为持有,单机模式需要配置
	sCfgTmp = ToString<int>(gc_nLoginTokenWait);
	if (0 == pConfig->GetProperty("init_token",sCfgTmp))
	{
		int nToken = FromString<int>(sCfgTmp);
		if (gc_nLoginTokenHold == nToken)
		{
			m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
			m_stLoginInfoLocal.tmToken = time(0);
		}
	}

	//是否需要初始化随机数
	sCfgTmp = "1";
	int nNeedInit = 1;
	if (0 == pConfig->GetProperty("need_init_magic",sCfgTmp))
	{
		nNeedInit = FromString<int>(sCfgTmp);		
	}
	if (0 != nNeedInit)
	{
		srand(static_cast<unsigned int>(time(0)));
		int RANGE_MIN = 0;
		int RANGE_MAX = 10000;
		m_stLoginInfoLocal.nMagicNum = rand() * (RANGE_MAX - RANGE_MIN) / RAND_MAX + RANGE_MIN;
	}


	CRLog(E_APPINFO,"主备信息:本节点ID:%d 主从标志:%d 随机数:%d",m_nNodeID,m_stLoginInfoLocal.nMasterSlave,m_stLoginInfoLocal.nMagicNum);
	return 0;
}

int CLoginMgr::Start()
{
	BeginThread();

	//注册登录状态等
	vector< pair<string,string> > vNmo;
	pair<string,string> pa;
	pa.first = gc_sSgeLoginState;
	pa.second = gc_sSgeLoginState + ".0";
	vNmo.push_back(pa);

	pa.first = gc_sSgeLoginToken;
	pa.second = gc_sSgeLoginToken + ".0";
	vNmo.push_back(pa);

	pa.first = gc_sSgeLoginInd;
	pa.second = gc_sSgeLoginInd + ".0";
	vNmo.push_back(pa);

	pa.first = gc_sSgeLoginAlarm;
	pa.second = gc_sSgeLoginAlarm + ".0";
	vNmo.push_back(pa);
	CNetMgr::Instance()->Register(&m_oNm, vNmo);

	CNMO oNmo;
	oNmo.m_sOid = gc_sSgeLoginAlarm;
	oNmo.m_sOidIns = gc_sSgeLoginAlarm + ".0";
	oNmo.m_nQuality = gc_nQuolityGood;
	oNmo.m_sTimeStamp = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");
	oNmo.m_sValue = ToString<int>(gc_nLoginNormal);
	CNetMgr::Instance()->Report(oNmo);
	return 0;
}

void CLoginMgr::Stop()
{
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
	m_csState.Unlock();

	EndThread();

	//网管注销
	CNetMgr::Instance()->UnRegisterModule(&m_oNm);
}

void CLoginMgr::Finish()
{
	m_deqMIf.clear();
	return;
}

int CLoginMgr::SendPacket(CPacket &GessPacket)
{
	try
	{
		CIpcPacket & pkt = dynamic_cast<CIpcPacket &>(GessPacket);
		
		m_deqCondMutex.Lock();
		m_deqMIf.push_back(pkt);
		m_deqCondMutex.Unlock();
		m_deqCondMutex.Signal();
		return 0;
	}
	catch(std::bad_cast)
	{
		CRLog(E_ERROR,"packet error!");
		return -1;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}

int CLoginMgr::OnRecvPacket(CPacket &GessPacket)
{
	if (0 != m_pCpMgr)
		return m_pCpMgr->Forward(GessPacket,m_ulKey);

	return -1;
}


//根据连接对端报盘机结果进行处理
void CLoginMgr::ConnectNtf(int nFlag)
{
	try
	{
		m_csState.Lock();
		if (0 != nFlag /*&& m_blFirstConnectState*/)
		{//首次连接对端报盘机且失败
			CRLog(E_APPINFO,"主备信息：对端连接失败!自动获得登录令牌");
			m_blFirstConnectState = false;

			//m_stLoginInfoLocal.nLoginIndication = gc_nIndLogin; // added by Ben 20110503
			m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
			m_stLoginInfoLocal.tmToken = time(0);
			bool blLogin = IsNeedLogin();
			m_csState.Unlock();	

			if (blLogin)
			{
				PrepareLogin();
			}
			return;
		}

		if (0 == nFlag)
		{
			m_nConnectState = gc_nStateConnected;
		}
		else
		{
			m_nConnectState = gc_nStateDisConnected;
		}

		if (m_blFirstConnectState)
			m_blFirstConnectState = false;
		
		if (0 == nFlag && !m_stLoginInfoLocal.blInfoSended)
		{
			CRLog(E_APPINFO,"主备信息:连接对端成功，发送本地登录信息!");
			m_stLoginInfoLocal.blInfoSended = true;
			SendStateInfo();
		}
		m_csState.Unlock();
	}
	catch(...)
	{
		m_csState.Unlock();
		CRLog(E_ERROR,"Unknown exception");
	}
}

//对端报盘机连接中断
void CLoginMgr::DisconnectNtf()
{
	CRLog(E_APPINFO,"主备信息：对端连接中断");

	bool blLogin = false;
	m_csState.Lock();
	m_nConnectState = gc_nStateDisConnected;
	if (gc_nLoginTokenWait == m_stLoginInfoLocal.nLoginToken)
	{
		CRLog(E_APPINFO,"主备信息：对端连接中断!自动获得登录令牌");
		m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
		m_stLoginInfoLocal.tmToken = time(0);
		blLogin = IsNeedLogin();
	}
	m_stLoginInfoLocal.blInfoSended = false;
	m_csState.Unlock();

	if (blLogin)
	{
		PrepareLogin();
	}
}

void CLoginMgr::AcceptNtf()
{
}

//准备登录
int CLoginMgr::PrepareLogin()
{
	unsigned int uiLeft = 2;
	CDataSrcCpMgr* pCpMgr =  dynamic_cast<CDataSrcCpMgr*>(m_pCpMgr);

	if (pCpMgr->Stock_Dll_Init())
	{
		OnLogin(gc_nStateLogined);
	}
	else
		OnLogin(0);

	return 0;
}

bool CLoginMgr::IsNeedLogout()
{
	bool blFlag = false;
	m_csState.Lock();
	if (gc_nStateLogined == m_stLoginInfoLocal.nLoginState)
	{
		blFlag = true;
	}
	m_csState.Unlock();

	return blFlag;
}

//登出指示,所有交易服务器中断指示登出
int CLoginMgr::LogoutInd()
{
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginIndication = gc_nIndLogout;

	//如果已经登录则登出
	if (!IsNeedLogout())
	{
		CRLog(E_APPINFO,"主备信息:不具备登出条件!");
		m_csState.Unlock();
		return -1;
	}

	CDataSrcCpMgr* pCpMgr =  dynamic_cast<CDataSrcCpMgr*>(m_pCpMgr);

	if (pCpMgr->Stock_Dll_Release())
	{
		OnLogout(4);
	}
	m_csState.Unlock();
	return 0;
}

//登录登出结果通知 报盘模块调用
void CLoginMgr::OnLogin(int nResult)
{
	CRLog(E_APPINFO,"主备信息:登录结果指示(%d)!",nResult);

	bool blNegotiate = false;
	bool blConnFlag = true;
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginResult = nResult;
	if (gc_nStateLogined == nResult)
	{
		m_stLoginInfoLocal.nLoginState = gc_nStateLogined;
		m_stLoginInfoLocal.nLoginIndication = gc_nIndLogin;
	}
	else if (gc_nStateLoginning != nResult && m_blTokenConflict)
	{
		CRLog(E_APPINFO,"主备信息:令牌冲突,放弃令牌,停止登录!");
		m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
		m_stLoginInfoLocal.nLoginState = gc_nStateLoginInit;
		m_stLoginInfoLocal.nLoginResult = gc_nStateLoginInit;
		blNegotiate = true;
	}
	blConnFlag = (gc_nStateConnected == m_nConnectState);
	m_csState.Unlock();

	//上报网管
	CNMO oNmo;
	oNmo.m_sOid = gc_sSgeLoginAlarm;
	oNmo.m_sOidIns = gc_sSgeLoginAlarm + ".0";
	oNmo.m_nQuality = gc_nQuolityGood;
	oNmo.m_sTimeStamp = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");

	if (blNegotiate)
	{
		 CRLog(E_APPINFO,"主备信息:令牌冲突,放弃令牌,重新协商!"); 
		 SendStateInfo();

		 oNmo.m_sValue = ToString<int>(gc_nLoginNormal);
	}
	else
	{
		if (gc_nStateLogined == nResult && blConnFlag)
		{
			CRLog(E_APPINFO,"主备信息:登录成功,发送登录信息!");	
			SendStateInfo();
		}

		oNmo.m_sValue = ToString<int>(GetAlarmStat());
	}

	CNetMgr::Instance()->Report(oNmo);
}

//登录登出结果通知 报盘模块调用
void CLoginMgr::OnLogout(int nResult)
{
	CRLog(E_APPINFO,"主备信息:登出结果:%d!",nResult);

	bool blConnFlag = true;
	m_csState.Lock();
	m_stLoginInfoLocal.nLoginResult = nResult;
	if (gc_nStateLogouted == nResult)
	{
		m_stLoginInfoLocal.nLoginState = gc_nStateLogouted;
	}
	
	blConnFlag = (gc_nStateConnected == m_nConnectState);
	//m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
	m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenGiveup;
	m_csState.Unlock();

	if (blConnFlag && gc_nStateLogoutting != nResult)
	{
		CRLog(E_APPINFO,"主备信息:登出,发送登录信息!");
		SendStateInfo();
	}
}


//处理M接口报文线程
int CLoginMgr::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqMIf.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			if ( !m_deqMIf.empty())
			{
				CIpcPacket pkt = m_deqMIf.front();
				m_deqMIf.pop_front();
				m_deqCondMutex.Unlock();

				try
				{
					RunPacketHandleApi(pkt);
				}
				catch(...)
				{
					CRLog(E_ERROR,"Unknown exception! ");	
				}
				continue;
			}
			m_deqCondMutex.Unlock();
		}

		CRLog(E_SYSINFO,"Trader_Backup Thread exit!");
		return 0;	
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s! m_bEndThread=%s",e.what(),m_bEndThread?"true":"false");
		return -1;
	}
	catch(...)
	{
		CRLog(E_ERROR,"Unknown exception! m_bEndThread=%s",m_bEndThread?"true":"false");		
		return -1;
	}
}

int CLoginMgr::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();
	Wait();
	return 0;
}

//主动类线程状态是否需要被网管
bool CLoginMgr::IsNetManaged(string& sKeyName)
{
	sKeyName = "报盘机主备管理线程";
	return true;
}

//令牌协商
bool CLoginMgr::HandleStateInfo(StateInfo& stBody,int& nResult)
{
	bool blNegotiate = false;

	m_csState.Lock();
	m_stLoginInfoRemote.nMasterSlave = FromString<int>(stBody.ms_flag);
	m_stLoginInfoRemote.nLoginIndication = FromString<int>(stBody.ind_login);
	m_stLoginInfoRemote.nLoginState = FromString<int>(stBody.login_state);
	m_stLoginInfoRemote.nLoginToken = FromString<int>(stBody.token);
	m_stLoginInfoRemote.tmToken = FromString<time_t>(stBody.tm_token);
	int nPeerNodeID = 0;
	int nPktFlag = FromString<int>(stBody.pkt_flag);
	if (gc_nPktRequest == nPktFlag)
	{
		nPeerNodeID = FromString<int>(stBody.node_id);
	}
	else
	{
		nPeerNodeID = FromString<int>(stBody.node_peer_id);
	}
	int nMagicNum = FromString<int>(stBody.magic_number);
	
	//初始化为无冲突，是否冲突每次协商重新进行判断
	m_blTokenConflict = false;

	if (gc_nLoginTokenHold != m_stLoginInfoRemote.nLoginToken)
	{
		if (gc_nLoginTokenHold != m_stLoginInfoLocal.nLoginToken)
		{//都未持有令牌
			if (gc_nLoginTokenGiveup == m_stLoginInfoRemote.nLoginToken)
			{
				CRLog(E_APPINFO,"主备信息:两端都未持有令牌,对端主动放弃,本端自动获得令牌!");
				m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
				m_stLoginInfoLocal.tmToken = time(0);
				nResult = gc_nTokenWait2Hold;
			}
			else if (gc_nMaster == m_stLoginInfoLocal.nMasterSlave && gc_nMaster != m_stLoginInfoRemote.nMasterSlave)
			{//对端未持有令牌 若本地为Master则持有令牌
				CRLog(E_APPINFO,"主备信息:两端都未持有令牌,本端配置为主,对端配置为从,本端自动获得令牌!");
				m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
				m_stLoginInfoLocal.tmToken = time(0);
				nResult = gc_nTokenWait2Hold;
			}
			else if (gc_nMaster != m_stLoginInfoLocal.nMasterSlave && gc_nMaster == m_stLoginInfoRemote.nMasterSlave)
			{
				CRLog(E_APPINFO,"主备信息:两端都未持有令牌,本端配置为从,对端配置为主,本端未获令牌!");
				nResult = gc_nTokenWait2Wait;
			}
			else if (gc_nMaster == m_stLoginInfoLocal.nMasterSlave && gc_nMaster == m_stLoginInfoRemote.nMasterSlave)
			{
				CRLog(E_APPINFO,"主备信息:配置冲突,两端都未持有令牌,两端都配置为主!");
				if (m_nNodeID > nPeerNodeID)
				{
					CRLog(E_APPINFO,"主备信息:本端节点号大于对端节点号,本端重新设置为从,未获令牌!");
					m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
					nResult = gc_nTokenWait2Wait;
				}
				else if (m_nNodeID < nPeerNodeID)
				{
					CRLog(E_APPINFO,"主备信息:本端节点号小于对端节点号,保持为主,获令牌!");
					m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
					m_stLoginInfoLocal.tmToken = time(0);
					nResult = gc_nTokenWait2Hold;
				}
				else
				{
					CRLog(E_APPINFO,"主备信息:本端节点号等于对端节点号,根据随即数进行主从决定!");
					if (nMagicNum < m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"主备信息:本端随机数大于对端随机数,本端重新设置为从,未获令牌!");
						m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
						nResult = gc_nTokenWait2Wait;
					}
					else if (nMagicNum > m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"主备信息:本端随机数小于对端随机数,保持为主,获令牌!");
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
						m_stLoginInfoLocal.tmToken = time(0);
						nResult = gc_nTokenWait2Hold;
					}
					else
					{
						CRLog(E_APPINFO,"主备信息:本端随机数等于对端随机数,重新生成随机数进行主从决定!");
						nResult = gc_nTokenWait2Wait;
						blNegotiate = true;
					}
				}
			}
			else
			{
				CRLog(E_APPINFO,"主备信息:配置冲突,两端都未持有令牌,两端都配置为从!");	
				if (m_nNodeID > nPeerNodeID)
				{
					CRLog(E_APPINFO,"主备信息:本端节点号大于对端节点号,本端保持为从,未获令牌!");
					m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
					nResult = gc_nTokenWait2Wait;
				}
				else if (m_nNodeID < nPeerNodeID)
				{
					CRLog(E_APPINFO,"主备信息:本端节点号小于对端节点号,重新设置为主,获令牌!");
					m_stLoginInfoLocal.nMasterSlave = gc_nMaster;
					m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
					m_stLoginInfoLocal.tmToken = time(0);
					nResult = gc_nTokenWait2Hold;
				}
				else
				{
					CRLog(E_APPINFO,"主备信息:本端节点号等于对端节点号,根据随即数进行主从决定!");
					if (nMagicNum < m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"主备信息:本端随机数大于对端随机数,本端保持为从,未获令牌!");
						m_stLoginInfoLocal.nMasterSlave = gc_nSlave;
						nResult = gc_nTokenWait2Wait;
					}
					else if (nMagicNum > m_stLoginInfoLocal.nMagicNum)
					{
						CRLog(E_APPINFO,"主备信息:本端随机数小于对端随机数,重新设置为主,获令牌!");
						m_stLoginInfoLocal.nMasterSlave = gc_nMaster;
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenHold;
						m_stLoginInfoLocal.tmToken = time(0);
						nResult = gc_nTokenWait2Hold;
					}
					else
					{
						CRLog(E_APPINFO,"主备信息:本端随机数等于对端随机数,重新生成随机数进行主从决定!");
						nResult = gc_nTokenWait2Wait;
						blNegotiate = true;
					}
				}
			}
		}
		else
		{
			CRLog(E_APPINFO,"主备信息:对端未持令牌,本端持令牌!");
			nResult = gc_nTokenHold2Hold;
		}
	}
	else
	{
		if (gc_nLoginTokenHold == m_stLoginInfoLocal.nLoginToken)
		{//冲突
			CRLog(E_APPINFO,"主备信息:冲突,两端同时持有令牌!");
			if (gc_nStateLogined != m_stLoginInfoLocal.nLoginState)
			{//本地未登录成功
				if (gc_nStateLogined != m_stLoginInfoRemote.nLoginState)
				{//对方未登录成功 
					if (gc_nIndLogin != m_stLoginInfoLocal.nLoginIndication)
					{//若本端尚无登录指示,则放弃令牌
						CRLog(E_APPINFO,"主备信息:本端无登录指示,本端放弃令牌,重新协商!");
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
						nResult = gc_nTokenHold2Wait;
						blNegotiate = true;
					}
					else
					{//本端已有登录指示,则标志冲突,重新协商
						CRLog(E_APPINFO,"主备信息:置令牌冲突标志,根据登录结果重新协商!");
						nResult = gc_nTokenHold2Hold;
						m_blTokenConflict = true;
					}
					//则比较获取令牌的时间和节点号大小
					//if (m_stLoginInfoLocal.tmToken == m_stLoginInfoRemote.tmToken)
					//{
					//	CRLog(E_APPINFO,"主备信息:两端获得令牌时间点一样!");
					//	if (m_nNodeID > nPeerNodeID)
					//	{
					//		CRLog(E_APPINFO,"主备信息:本端节点号大于对端节点号,本端放弃令牌!");
					//		m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					//	}
					//	else if (m_nNodeID < nPeerNodeID)
					//	{
					//		CRLog(E_APPINFO,"主备信息:本端节点号小于对端节点号,本端继续持有令牌!");
					//	}
					//	else
					//	{
					//		CRLog(E_APPINFO,"主备信息:本端节点号等于对端节点号,根据随即数进行主从决定!");
					//		if (nMagicNum < m_stLoginInfoLocal.nMagicNum)
					//		{
					//			CRLog(E_APPINFO,"主备信息:本端随机数大于对端随机数,本端放弃令牌!");
					//			m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					//		}
					//		else if (nMagicNum > m_stLoginInfoLocal.nMagicNum)
					//		{
					//			CRLog(E_APPINFO,"主备信息:本端随机数小于对端随机数,本端继续持有令牌!");
					//		}
					//		else
					//		{
					//			CRLog(E_APPINFO,"主备信息:本端随机数等于对端随机数,重新生成随机数进行主从决定!");
					//			blMagicNum = true;
					//		}
					//	}
					//}
					//else if (m_stLoginInfoLocal.tmToken < m_stLoginInfoRemote.tmToken)
					//{
					//	CRLog(E_APPINFO,"主备信息:本端获得令牌时间比对端早,本端继续持有令牌");
					//}
					//else
					//{
					//	CRLog(E_APPINFO,"主备信息:对端获得令牌时间比本端早,本端放弃令牌");
					//	m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					//}
				}
				else
				{//对方已登录成功
					if (gc_nIndLogin != m_stLoginInfoLocal.nLoginIndication)
					{//若本端尚无登录指示,则放弃令牌
						CRLog(E_APPINFO,"主备信息:本端无登录指示,本端放弃令牌,重新协商!");
						m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
						nResult = gc_nTokenHold2Wait;
						blNegotiate = true;
					}
					else
					{//本端已有登录指示,则标志冲突,重新协商
						CRLog(E_APPINFO,"主备信息:置令牌冲突标志,根据登录结果重新协商!");
						nResult = gc_nTokenHold2Hold;
						m_blTokenConflict = true;
					}
				}
			}
			else
			{
				if (gc_nStateLogined != m_stLoginInfoRemote.nLoginState)
				{//目前永远不进此逻辑
					CRLog(E_APPINFO,"主备信息:本端已登录成功,继续持有令牌!");
					nResult = gc_nTokenHold2Hold;
				}
				else
				{
					CRLog(E_APPINFO,"主备信息:两端已登录成功,重新协商!");
					//????
					m_stLoginInfoLocal.nLoginToken = gc_nLoginTokenWait;
					nResult = gc_nTokenHold2Wait;
					m_blTokenConflict = true;
					blNegotiate = true;
					//退出
				}
			}
		}
		else
		{
			CRLog(E_APPINFO,"主备信息:对端持有令牌,本端未持令牌!");
			nResult = gc_nTokenWait2Wait;
		}
	}
	
	m_stLoginInfoLocal.blInfoSended = true;
	m_csState.Unlock();
	return blNegotiate;
}

//处理对端状态信息报文
int CLoginMgr::OnStateInfo(CIpcPacket& pkt)
{
	StateInfo stBody;
	CPacketStructIpcOffer::Packet2Struct(stBody, pkt);
	CRLog(E_APPINFO,"接收状态信息::对端节点ID(%s),主从标志(%s),登录指示(%s),登录状态(%s),令牌(%s),令牌时间(%s),随机数(%s)",stBody.node_id.c_str(),stBody.ms_flag.c_str(),stBody.ind_login.c_str(),stBody.login_state.c_str(),stBody.token.c_str(),stBody.tm_token.c_str(),stBody.magic_number.c_str());

	//是否需要登录
	bool blLogout = false;
	bool blLogin = false;
	int nResult = gc_nTokenWait2Wait;	
	m_csState.Lock();
	bool blNegotiate = HandleStateInfo(stBody,nResult);
	if (gc_nTokenWait2Hold == nResult)
	{
		blLogin = true;
	}
	else if (gc_nTokenHold2Wait == nResult)
	{
		//退出
		blLogout = true;
	}

	StateInfo stBodyRsp;
	stBodyRsp.node_id = stBody.node_id;
	stBodyRsp.ms_flag = ToString<int>(m_stLoginInfoLocal.nMasterSlave);
	stBodyRsp.ind_login = ToString<int>(m_stLoginInfoLocal.nLoginIndication);
	stBodyRsp.login_state = ToString<int>(m_stLoginInfoLocal.nLoginState);
	stBodyRsp.token = ToString<int>(m_stLoginInfoLocal.nLoginToken);
	stBodyRsp.tm_token = ToString<time_t>(m_stLoginInfoLocal.tmToken);
	stBodyRsp.magic_number = ToString<int>(m_stLoginInfoLocal.nMagicNum);
	stBodyRsp.pkt_flag = ToString<int>(gc_nPktResponse);
	stBodyRsp.node_peer_id = ToString<int>(m_nNodeID);
	m_csState.Unlock();

	//发送响应
	CIpcPacket pktRsp("StateRsp");
	CPacketStructIpcOffer::Struct2Packet(stBodyRsp, pktRsp);
	if (0 != m_pCpMgr)
		m_pCpMgr->Forward(pktRsp,m_ulKey);

	CRLog(E_APPINFO,"发送状态响应:本端节点ID(%d),主从标志(%s),登录指示(%s),登录状态(%s),令牌(%s),令牌时间(%s),随机数(%s)",m_nNodeID,stBodyRsp.ms_flag.c_str(),stBodyRsp.ind_login.c_str(),stBodyRsp.login_state.c_str(),stBodyRsp.token.c_str(),stBodyRsp.tm_token.c_str(),stBodyRsp.magic_number.c_str());
	
	//如果需要进行登录
	if (blLogin)
	{
		PrepareLogin();
	}

	if (blLogout)
	{
		LogoutInd();
	}
	return 0;
}

//接收到的状态应答
int CLoginMgr::OnStateInfoRsp(CIpcPacket& pktRsp)
{
	StateInfo stBody;
	CPacketStructIpcOffer::Packet2Struct(stBody, pktRsp);
	CRLog(E_APPINFO,"接收状态响应::对端节点ID(%s),主从标志(%s),登录指示(%s),登录状态(%s),令牌(%s),令牌时间(%s),随机数(%s)",stBody.node_peer_id.c_str(),stBody.ms_flag.c_str(),stBody.ind_login.c_str(),stBody.login_state.c_str(),stBody.token.c_str(),stBody.tm_token.c_str(),stBody.magic_number.c_str());

	bool blNegotiate = false;
	bool blLogin = false;
	int nResult = gc_nTokenWait2Wait;
	m_csState.Lock();
	blNegotiate = HandleStateInfo(stBody,nResult);
	if (gc_nTokenWait2Hold == nResult)
	{
		blLogin = true;
	}
	m_csState.Unlock();

	//如果需要进行登录
	if (blLogin)
	{
		PrepareLogin();
	}
	else if (blNegotiate)
	{//重新生成随机数,重新协商
		SendStateInfo(blNegotiate);
	}
	return 0;
}

//发送登录状态信息
int CLoginMgr::SendStateInfo(bool blMagicGen)
{
	CIpcPacket pkt("StateInfo");
	StateInfo stBody;
	stBody.node_id = ToString<int>(m_nNodeID);
	
	//生成随机数
	int nMagicNum = 0;
	if (blMagicGen)
	{
		srand(static_cast<unsigned int>(time(0)));
		int RANGE_MIN = 0;
		int RANGE_MAX = 10000;
		nMagicNum = rand() * (RANGE_MAX - RANGE_MIN) / RAND_MAX + RANGE_MIN;
	}

	m_csState.Lock();
	if (blMagicGen)
	{
		m_stLoginInfoLocal.nMagicNum = nMagicNum;
	}
	stBody.ms_flag = ToString<int>(m_stLoginInfoLocal.nMasterSlave);
	stBody.ind_login = ToString<int>(m_stLoginInfoLocal.nLoginIndication);
	stBody.login_state = ToString<int>(m_stLoginInfoLocal.nLoginState);
	stBody.token = ToString<int>(m_stLoginInfoLocal.nLoginToken);
	stBody.tm_token = ToString<time_t>(m_stLoginInfoLocal.tmToken);
	stBody.magic_number = ToString<int>(m_stLoginInfoLocal.nMagicNum);
	stBody.pkt_flag = ToString<int>(gc_nPktRequest);
	m_csState.Unlock();

	CPacketStructIpcOffer::Struct2Packet(stBody, pkt);
	if (0 != m_pCpMgr)
		m_pCpMgr->Forward(pkt,m_ulKey);

	CRLog(E_APPINFO,"发送状态信息:本端节点ID(%s),主从标志(%s),登录指示(%s),登录状态(%s),令牌(%s),令牌时间(%s),随机数(%s)",stBody.node_id.c_str(),stBody.ms_flag.c_str(),stBody.ind_login.c_str(),stBody.login_state.c_str(),stBody.token.c_str(),stBody.tm_token.c_str(),stBody.magic_number.c_str());
	return 0;
}

//是否已经登录
bool CLoginMgr::IsLogined()
{
	bool blFlag = false;
	m_csState.Lock();
	if (gc_nStateLogined == m_stLoginInfoLocal.nLoginResult)
	{
		blFlag = true;
	}
	m_csState.Unlock();
	return blFlag;
}
bool CLoginMgr::IsNeedLogin()
{
	bool blFlag = false;
	m_csState.Lock();
	if (gc_nLoginTokenHold == m_stLoginInfoLocal.nLoginToken && 
		/*gc_nIndLogin == m_stLoginInfoLocal.nLoginIndication && */
		gc_nStateLogined != m_stLoginInfoLocal.nLoginState)
	{
		blFlag = true;
	}
	m_csState.Unlock();

	return blFlag;
}

//网管查询
int CLoginMgr::LoginQuery(CNMO& oNmo) const
{
	oNmo.m_nQuality = gc_nQuolityUncertain;
	oNmo.m_sTimeStamp = CGessDate::NowToString("-") + " " + CGessTime::NowToString(":");
	if (oNmo.m_sOid == gc_sSgeLoginState)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		//oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginState);
		oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginResult);
	}
	else if(oNmo.m_sOid == gc_sSgeLoginToken)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginToken);
	}
	else if(oNmo.m_sOid == gc_sSgeLoginInd)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		oNmo.m_sValue = ToString<int>(m_stLoginInfoLocal.nLoginIndication);
	}
	else if(oNmo.m_sOid == gc_sSgeLoginAlarm)
	{
		oNmo.m_nQuality = gc_nQuolityGood;
		int nAlmStat = GetAlarmStat();
		oNmo.m_sValue = ToString<int>(nAlmStat);
	}
	return 0;
}

//是否告警状态
int CLoginMgr::GetAlarmStat() const
{
	int nAlm = gc_nLoginNormal;
	m_csState.Lock();
	if (gc_nLoginTokenHold == m_stLoginInfoLocal.nLoginToken 
		&& gc_nIndLogin == m_stLoginInfoLocal.nLoginIndication 
		&& gc_nStateLogined != m_stLoginInfoLocal.nLoginState)
	{
		nAlm = gc_nLoginAlarm;
	}
	m_csState.Unlock();
	return nAlm;
}