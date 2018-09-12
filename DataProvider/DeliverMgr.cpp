#include "DeliverMgr.h"
#include "ProviderCpMgr.h"
#include "HisDataHandler.h"
#include "MibConstant.h"

using namespace MibConst;

#define GBYTES	(1024 * 1024 * 1024)
#define MBYTES	(1024 * 1024)

CDeliverMgr::CDeliverMgr(void)
:m_pProviderCpMgr(0)
,m_pCfg(0)
,m_uiFwdCount(0)
,m_uiFwdGBytes(0)
,m_uiFwdBytes(0)
,m_uiFwdGBytesLast(0)
,m_uiFwdBytesLast(0)
,m_nInterval(0)
,m_uiMaxBandWidth(0)
,m_uiMinBandWidth(0)
,m_uiAvgBandWidth(0)
,m_uiLastBandWidth(0)
,m_uiQuoPktCount(0)
,m_uiQuoPktBytes(0)
,m_uiQuoPktGBytes(0)
,m_uiOrderFwdCount(0)
,m_uiOrderFwdBytes(0)
,m_ucMaxPerPkt(8)
{
}

CDeliverMgr::~CDeliverMgr(void)
{
}

//初始化配置 
int CDeliverMgr::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	//CConfig* pConfig = m_pCfg->GetCfgGlobal();

	string sMaxPkts;
	if (0 == m_pCfg->GetProperty("max_quotation_per_pkt", sMaxPkts))
	{
		unsigned int uiTmp = strutils::FromString<unsigned int>(sMaxPkts);
		if (uiTmp > 0xFF)
		{
			m_ucMaxPerPkt = 8;
		}
		else if (uiTmp < 1)
		{
			m_ucMaxPerPkt = 8;
		}
		else
		{
			m_ucMaxPerPkt = static_cast<unsigned char>(uiTmp);
		}
	}

	string sTmp;
	unsigned int uiInitFromLast = 0;
	if (0 == m_pCfg->GetProperty("init_from_last", sTmp))
	{
		uiInitFromLast = strutils::FromString<unsigned int>(sTmp);
		if (uiInitFromLast != 0 && uiInitFromLast != 1)
		{
			uiInitFromLast = 0;
		}
	}

	m_sFileLast = ".\\hisdata\\last.dat";
	if (1 == uiInitFromLast)
	{
		ifstream ifsLast;
		ifsLast.open(m_sFileLast.c_str(),ios::binary | ios::in);
		if (ifsLast.is_open())
		{
			while (!ifsLast.eof())
			{
				QUOTATION stQuotation = {0};
				ifsLast.read((char*)(&stQuotation), sizeof(QUOTATION));
				if (ifsLast.eof())
					break;

				string sKey = stQuotation.Key();
				m_mapQuotation.insert(pair<string, QUOTATION>(sKey, stQuotation));
			}
			ifsLast.close();
		}
	}

	std::string sUnitFile;
	sUnitFile = DEFUALT_CONF_PATH PATH_SLASH;
	sUnitFile = sUnitFile + "Unit";
	sUnitFile = sUnitFile + ".cfg";
	
	m_oUnitFile.Load(sUnitFile);
	vector<string> vUnit = m_oUnitFile.GetKeys();
	for (vector<string>::iterator it = vUnit.begin(); it != vUnit.end(); ++it)
	{
		string sTmp;
		if (0 == m_oUnitFile.GetProperty(*it, sTmp))
		{
			unsigned int uiVal = FromString<unsigned int>(sTmp);
			if (uiVal == 0)
				uiVal = 1;

			m_mapUnit[*it] = uiVal;
		}
	}

	//
	m_oNmoModule.Bind(this);
	CNetMgr::Instance()->Register(&m_oNmoModule,mibQueNum,mibQueNum+"."+"DeliverMgr队列");

	vector< pair<string,string> > vpaOid;
	pair<string,string> pa;
	string sNmKey = "0";
	
	pa.first = gc_sFwdCount;
	pa.second = gc_sFwdCount + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sQuoPktMBytes;
	pa.second = gc_sQuoPktMBytes + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sNowBandWidth;
	pa.second = gc_sNowBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sMaxBandWidth;
	pa.second = gc_sMaxBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sMinBandWidth;
	pa.second = gc_sMinBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sAvgBandWidth;
	pa.second = gc_sAvgBandWidth + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sQuoPerPkt;
	pa.second = gc_sQuoPerPkt + "." + sNmKey;
	vpaOid.push_back(pa);


	pa.first = gc_sBytesPerPkt;
	pa.second = gc_sBytesPerPkt + "." + sNmKey;
	vpaOid.push_back(pa);

	pa.first = gc_sSubscribers;
	pa.second = gc_sSubscribers + "." + sNmKey;
	vpaOid.push_back(pa);

	CNetMgr::Instance()->Register(&m_oNmoModule,vpaOid);
	return 0;
}

//启动线程 
int CDeliverMgr::Start()
{
	//启动调度线程
	BeginThread();
	return 0;
}

//停止线程
void CDeliverMgr::Stop()
{
	//停止调度线程
	CRLog(E_APPINFO,"%s","Stop DeliverMgr Thread");
	EndThread();
}

//清理资源
void CDeliverMgr::Finish()
{
	m_deqQuotation.clear();
	m_deqOrder.clear();
	m_mmapQuotationOrder.clear();
	m_mapUnit.clear();

	//清理缓存行情数据
	ofstream m_ofsLast;
	m_ofsLast.open(m_sFileLast.c_str(),ios::binary | ios::out | ios::trunc);
	if (m_ofsLast.is_open())
	{
		for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
		{
			m_ofsLast.write((const char*)(&(*it).second), sizeof(QUOTATION));
		}
		m_ofsLast.close();
	}
	m_mapQuotation.clear();

	CNetMgr::Instance()->UnRegisterModule(&m_oNmoModule);
}

void CDeliverMgr::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pProviderCpMgr = dynamic_cast<CProviderCpMgr*>(pCpMgr);
}


int CDeliverMgr::SendPacket(CPacket &pkt)
{
	try
	{
		
		CSamplerPacket & pktOrder = dynamic_cast<CSamplerPacket&>(pkt);	

		m_deqCondMutex.Lock();
		m_deqOrder.push_back(pktOrder);
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

///行情信息入队列
int CDeliverMgr::Enque(QUOTATION& stQuotation)
{
	try
	{
		m_deqCondMutex.Lock();
		m_deqQuotation.push_back(stQuotation);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		return 0;	
	}
	catch (std::exception e)
	{
		CRLog(E_ERROR, "CDeliverMgr::Enque exception:%s",e.what()); 
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"CDeliverMgr::Enque Unknown exception");
		return -1;
	}
}

int CDeliverMgr::Query(CNMO& oNmo)
{
	int nRtn = 0;
	oNmo.m_nQuality=gc_nQuolityGood;

	if(oNmo.m_sOidIns==(mibQueNum+"."+"DeliverMgr队列"))
	{
		oNmo.m_sValue=ToString(m_deqQuotation.size());
	}
	else if (oNmo.m_sOid == gc_sFwdCount)
	{
		oNmo.m_sValue =  ToString<unsigned int>(m_uiFwdCount);
	}
	else if (oNmo.m_sOid == gc_sQuoPktMBytes)
	{
		oNmo.m_sValue =  ToString<unsigned int>(m_uiQuoPktGBytes*1024+m_uiQuoPktBytes/MBYTES);
	}
	else if (oNmo.m_sOid == gc_sNowBandWidth)
	{
		oNmo.m_sValue =  ToString<unsigned int>(m_uiLastBandWidth);
	}
	else if (oNmo.m_sOid == gc_sMaxBandWidth)
	{
		oNmo.m_sValue =  ToString<unsigned int>(m_uiMaxBandWidth);
	}
	else if (oNmo.m_sOid == gc_sMinBandWidth)
	{
		oNmo.m_sValue =  ToString<unsigned int>(m_uiMinBandWidth);
	}
	else if (oNmo.m_sOid == gc_sAvgBandWidth)
	{
		oNmo.m_sValue =  ToString<unsigned int>(m_uiAvgBandWidth);
	}
	else if (oNmo.m_sOid == gc_sQuoPerPkt)
	{
		double dlQuoPerPkt = 0.0;
		if (m_uiFwdCount != 0)
		{
			dlQuoPerPkt = (m_uiQuoPktCount*1.0)/m_uiFwdCount;
			oNmo.m_sValue =  ToString<double>(dlQuoPerPkt);
		}		
	}
	else if (oNmo.m_sOid == gc_sBytesPerPkt)
	{
		unsigned int uiBytesPerPkt = 0;
		if (m_uiQuoPktCount != 0)
		{
			uiBytesPerPkt = (m_uiQuoPktGBytes*GBYTES+m_uiQuoPktBytes)/m_uiQuoPktCount;
			oNmo.m_sValue =  ToString<unsigned int>(uiBytesPerPkt);
		}		
	}
	else if (oNmo.m_sOid == gc_sSubscribers)
	{
		map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();
		oNmo.m_sValue =  ToString<unsigned int>(mapSubscri.size());
	}
	return nRtn;
}

#define FOREIGN_MARKET	0x8000  // 外汇
#define WH_BASE_RATE    0x0100	// 基本汇率
int CDeliverMgr::ThreadEntry()
{
	try
	{
		//已组行情包数量,
		unsigned char cPkt = 0;
		//已组行情包字节序列
		string sQuotationVal;
		//上一个行情包所属市场类型
		unsigned int uiLastMarketType = (FOREIGN_MARKET|WH_BASE_RATE);
		uiLastMarketType &= 0x0000FFFF;

		while(!m_bEndThread)
		{
			Sleep(50);
			m_deqCondMutex.Lock();
			while(m_deqQuotation.empty() && m_deqOrder.empty() && m_mmapQuotationOrder.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();


			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}
			
			if (!m_deqOrder.empty() && !m_deqQuotation.empty())
			{//既有行情订阅也有行情包
				//为了使用引用，没有立即pop_front
				CSamplerPacket& oOrder = m_deqOrder.front();
				QUOTATION& stQuotation = m_deqQuotation.front();
				m_deqCondMutex.Unlock();

				HandleQuotationOrder(oOrder);

				//行情包在HandleQuotationDlv出队列
				HandleQuotationDlv(stQuotation, cPkt, sQuotationVal, uiLastMarketType);				

				m_deqCondMutex.Lock();
				m_deqOrder.pop_front();
				m_deqCondMutex.Unlock();				
			}
			else if (!m_deqOrder.empty() && m_deqQuotation.empty())
			{//有行情订阅 没有行情包 处理完订阅并开始发送
				CSamplerPacket& oOrder = m_deqOrder.front();
				m_deqCondMutex.Unlock();

				HandleQuotationOrder(oOrder);

				m_deqCondMutex.Lock();
				m_deqOrder.pop_front();
				m_deqCondMutex.Unlock();

				if (!m_mmapQuotationOrder.empty())
				{
					MMAP_IT it = m_mmapQuotationOrder.begin();
					HandleFirstQuotation((*it).first, (*it).second);
					m_mmapQuotationOrder.erase(it);

					if (m_mmapQuotationOrder.empty())
					{
						CRLog(E_STATICS,"Fwd Order pkts %u, %uk", m_uiOrderFwdCount, m_uiOrderFwdBytes / 1024);
					}
				}
			}
			else if (m_deqOrder.empty() && !m_deqQuotation.empty())
			{//只有行情包				
				QUOTATION& stQuotation = m_deqQuotation.front();
				m_deqCondMutex.Unlock();

				//行情包在HandleQuotationDlv出队列
				HandleQuotationDlv(stQuotation, cPkt, sQuotationVal, uiLastMarketType);
			}
			else
			{//目前没有新行情需要发送，而且存在没有发送完成的由于初次订阅载入的完整字段行情
				m_deqCondMutex.Unlock();

				if (!m_mmapQuotationOrder.empty())
				{
					MMAP_IT it = m_mmapQuotationOrder.begin();
					HandleFirstQuotation((*it).first, (*it).second);
					m_mmapQuotationOrder.erase(it);

					if (m_mmapQuotationOrder.empty())
					{
						CRLog(E_STATICS,"Fwd Order pkts %u, %uk", m_uiOrderFwdCount, m_uiOrderFwdBytes / 1024);
					}
				}
			}
		}
		CRLog(E_APPINFO,"%s","TranslateorHandler Thread exit!");
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

//目前只处理行情订阅和取消订阅报文
int CDeliverMgr::HandleQuotationOrder(CSamplerPacket& oOrder)
{
	try
	{
		string sCmdID = oOrder.GetCmdID();
		if (strutils::ToHexString<unsigned int>(YL_SUBSCRIP) == sCmdID)
		{
			HandleOrder(oOrder);
		}
		else if (strutils::ToHexString<unsigned int>(YL_UNSUBSCRIP) == sCmdID)
		{
			HandleCancelOrder(oOrder);
		}
		else
		{

		}
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

//行情订阅
int CDeliverMgr::HandleOrder(CSamplerPacket& oOrder)
{
	try
	{
		CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oOrder.GetMsg());

		char cSubscripType='0';
		vector<unsigned int> vecMarkeType;
		string sSubItems;
		
		unsigned int uiNodeID = 0;
		msg.GetField(MSG_NODE_ID,uiNodeID);
		unsigned int uiSeqNo = 0;
		msg.GetField(MSG_SEQ_ID,uiSeqNo);
		msg.GetField(MSG_SUBSCRIP_TYPE,sSubItems);
		cSubscripType = sSubItems.at(0);
		msg.GetBinaryField(MSG_SUBSCRIP_RECS,sSubItems);

		string sLog;
		size_t nDataLen = sSubItems.length();
		const unsigned int * puiData = (const unsigned int *)sSubItems.data();
		for(int i = 0;i < nDataLen/sizeof(unsigned int);i++)
		{
			unsigned int uiTmp = ntohl(*puiData);
			vecMarkeType.push_back(uiTmp);
			puiData ++;

			if (!sLog.empty())
			{
				sLog += ",";
			}
			sLog += ToHexString<unsigned int>(uiTmp);
		}

		if (vecMarkeType.size() == 1 && vecMarkeType[0] == 0xFFFFFFFF)
		{
			for (map<string, QUOTATION>::iterator itQuo = m_mapQuotation.begin(); itQuo != m_mapQuotation.end(); ++itQuo)
			{
				m_mmapQuotationOrder.insert(pair<string,unsigned int>((*itQuo).first, (unsigned int)uiNodeID));
			}
		}
		else
		{
			if (cSubscripType == '0')
			{
				for (vector<unsigned int>::iterator itType = vecMarkeType.begin(); itType != vecMarkeType.end(); ++itType)
				{
					for (map<string, QUOTATION>::iterator itQuo = m_mapQuotation.begin(); itQuo != m_mapQuotation.end(); ++itQuo)
					{
						if ((*itQuo).second.m_CodeInfo.m_usMarketType == *itType)
							m_mmapQuotationOrder.insert(pair<string,unsigned int>((*itQuo).first, uiNodeID));
					}
				}
			}
			else
			{
				for (map<string, QUOTATION>::iterator itQuo = m_mapQuotation.begin(); itQuo != m_mapQuotation.end(); ++itQuo)
				{
					bool blFind = false;
					for (vector<unsigned int>::iterator itType = vecMarkeType.begin(); itType != vecMarkeType.end(); ++itType)
					{					
						if ((*itQuo).second.m_CodeInfo.m_usMarketType == *itType)
						{
							blFind = true;
							break;
						}						
					}

					if (!blFind)
						m_mmapQuotationOrder.insert(pair<string,unsigned int>((*itQuo).first, uiNodeID));
				}
			}
		}

		CRLog(E_PROINFO,"OnRecvSubscrip NodeID:%u ,mode:%d,%s",uiNodeID, cSubscripType, sLog.c_str());

		///订阅时增加订阅请求
		CMemData::Instance()->GetSubscriberTbl().AddSubscripItem(uiNodeID,cSubscripType,vecMarkeType);

		CMessageImpl oMsgRsp;
		oMsgRsp.SetField(MSG_SEQ_ID,uiSeqNo);
		oMsgRsp.SetField(MSG_NODE_ID,uiNodeID);

		unsigned int nRst = 0;
		oMsgRsp.SetField(MSG_SUBSCRIP_RESULT,nRst);

		CSamplerPacket oPktRsp(oMsgRsp,YL_SUBSCRIP_RSP);
		if (0 != m_pProviderCpMgr)
			return m_pProviderCpMgr->Forward(oPktRsp,m_ulKey);

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

//取消行情订阅
int CDeliverMgr::HandleCancelOrder(CSamplerPacket& pkt)
{
	try
	{
		CMessageImpl& msg = dynamic_cast<CMessageImpl&>(pkt.GetMsg());

		vector<unsigned int> vecMarkeType;
		string sSubItems;
		unsigned int uiSeqNo = 0;
		msg.GetField(MSG_SEQ_ID,uiSeqNo);

		unsigned int uiNodeID = 0;
		msg.GetField(MSG_NODE_ID,uiNodeID);
		msg.GetBinaryField(MSG_SUBSCRIP_RECS,sSubItems);


		size_t nDataLen = sSubItems.length();
		const unsigned int * puiData = (const unsigned int *)sSubItems.data();
		for(int i = 0;i < nDataLen/sizeof(unsigned int);i++)
		{
			vecMarkeType.push_back(ntohl(*puiData));
			puiData ++;
		}

		CRLog(E_PROINFO,"OnRecvUnSubscrip SeqNo:%u, NodeID:%u ",uiSeqNo, uiNodeID);

		///订阅时增加订阅请求
		CMemData::Instance()->GetSubscriberTbl().CancelSubscriber(uiNodeID);


		CMessageImpl oMsgRsp;
		oMsgRsp.SetField(MSG_SEQ_ID,uiSeqNo);
		oMsgRsp.SetField(MSG_NODE_ID,uiNodeID);

		unsigned int nRst = 0;
		oMsgRsp.SetField(MSG_SUBSCRIP_RESULT,nRst);

		CSamplerPacket oPktRsp(oMsgRsp,YL_UNSUBSCRIP_RSP);
		if (0 != m_pProviderCpMgr)
			return m_pProviderCpMgr->Forward(oPktRsp,m_ulKey);

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

//处理行情发布
int CDeliverMgr::HandleQuotationDlv(QUOTATION& stQuotation, unsigned char& cPkt, string& sQuotationVal,unsigned int& uiLastMarketType)
{
	unsigned int uiMarketType = stQuotation.m_CodeInfo.m_usMarketType;	
	uiMarketType &= 0x0000FFFF;
	string sKey;
	
	try
	{	
		sKey = stQuotation.Key();
		if (!m_mmapQuotationOrder.empty())
		{//需要发送完整行情字段的第一次订阅表里 若存在当前合约，首先发送此合约完整行情字段给订阅者
		 //此表只在有新客户端订阅时才会载入数据，发送完成后清除
			bool blFlag = false;
			MMAP_IT it;
			RANGE_ORDER range = m_mmapQuotationOrder.equal_range(sKey);
			for (it = range.first; it != range.second; ++it)
			{
				HandleFirstQuotation(sKey,(*it).second);
				blFlag = true;
			}

			if (blFlag)
			{
				m_mmapQuotationOrder.erase(sKey);

				if (m_mmapQuotationOrder.empty())
				{
					CRLog(E_STATICS,"Fwd Order pkts %u, %uk", m_uiOrderFwdCount, m_uiOrderFwdBytes / 1024);
				}
			}
		}

		if (uiLastMarketType != uiMarketType)
		{//单个报文只能存放相同市场类型行情，立即发送
			if (!sQuotationVal.empty())
			{
				if (cPkt > 1)
				{
					string sTmp;
					sTmp.append(1,cPkt);
					sQuotationVal.replace(sizeof(uiLastMarketType),1,sTmp);
				}

				DelieveQuotation(uiLastMarketType, sQuotationVal);

				m_uiQuoPktCount += cPkt;
				m_uiQuoPktBytes += sQuotationVal.length() - sizeof(SAMPLER_PKT_HEADER) - 2 - 1 - cPkt;
				if (m_uiQuoPktBytes >= GBYTES)
				{
					m_uiQuoPktGBytes++;
					m_uiQuoPktBytes -= GBYTES;
				}

				cPkt = 0;
				sQuotationVal.clear();
			}
			uiLastMarketType = uiMarketType;
		}

		
		cPkt++;
		if (cPkt <= 1)
		{
			//同一市场类型的多个行情可以组到一个报文
			unsigned int uiTmp = htonl(uiLastMarketType);
			sQuotationVal.append((const char*)&uiTmp,sizeof(uiTmp));
			sQuotationVal.append(1,cPkt);
		}

		bitset<FIELDKEY_UNKNOWN> bsQuotation;
		FindDifference(stQuotation, bsQuotation);
		AssemblePkt(stQuotation, sQuotationVal, bsQuotation);
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
	}

	try
	{
		m_deqCondMutex.Lock();
		m_deqQuotation.pop_front();
		size_t nSize = m_deqQuotation.size();
		m_deqCondMutex.Unlock();

		//已经打包处理的行情包大于配置值或队列已经为空
		if (cPkt >= m_ucMaxPerPkt || nSize == 0)
		{
			if (!sQuotationVal.empty())
			{
				if (cPkt > 1)
				{
					string sTmp;
					sTmp.append(1,cPkt);
					sQuotationVal.replace(sizeof(uiLastMarketType),1,sTmp);
				}

				DelieveQuotation(uiMarketType, sQuotationVal);

				m_uiQuoPktCount += cPkt;
				m_uiQuoPktBytes += sQuotationVal.length() - sizeof(SAMPLER_PKT_HEADER) - 2 - 1 - cPkt;
				if (m_uiQuoPktBytes >= GBYTES)
				{
					m_uiQuoPktGBytes++;
					m_uiQuoPktBytes -= GBYTES;
				}

				cPkt = 0;
				sQuotationVal.clear();
			}
		}

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

//比较增量，并保存
int CDeliverMgr::FindDifference(QUOTATION& stQuotation, bitset<FIELDKEY_UNKNOWN>& bsQuotation)
{
	try
	{
		bsQuotation.reset();
		string sKey = stQuotation.Key();
		
		map<string, QUOTATION>::iterator it = m_mapQuotation.find(sKey);
		if (it != m_mapQuotation.end())
		{
			if ((*it).second.m_uilastClose != stQuotation.m_uilastClose)
			{
				bsQuotation.set(FIELDKEY_LASTCLOSE);
			}

			if ((*it).second.m_uiLastSettle != stQuotation.m_uiLastSettle)
			{
				bsQuotation.set(FIELDKEY_LASTSETTLE);
			}

			if ((*it).second.m_uiSettle != stQuotation.m_uiSettle)
			{
				bsQuotation.set(FIELDKEY_SETTLE);
			}

			if ((*it).second.m_uiOpenPrice != stQuotation.m_uiOpenPrice)
			{
				bsQuotation.set(FIELDKEY_OPEN);
			}

			if ((*it).second.m_uiHigh != stQuotation.m_uiHigh)
			{
				bsQuotation.set(FIELDKEY_HIGH);
			}

			if ((*it).second.m_uiLow != stQuotation.m_uiLow)
			{
				bsQuotation.set(FIELDKEY_LOW);
			}

			if ((*it).second.m_uiClose != stQuotation.m_uiClose)
			{
				bsQuotation.set(FIELDKEY_CLOSE);
			}

			if ((*it).second.m_uiHighLimit != stQuotation.m_uiHighLimit)
			{
				bsQuotation.set(FIELDKEY_HIGHLIMIT);
			}

			if ((*it).second.m_uiLowLimit != stQuotation.m_uiLowLimit)
			{
				bsQuotation.set(FIELDKEY_LOWLIMIT);
			}

			if ((*it).second.m_uiLast != stQuotation.m_uiLast)
			{
				bsQuotation.set(FIELDKEY_LAST);
			}

			if ((*it).second.m_uiAverage != stQuotation.m_uiAverage)
			{
				bsQuotation.set(FIELDKEY_AVERAGE);
			}

			if ((*it).second.m_uiVolume != stQuotation.m_uiVolume)
			{
				bsQuotation.set(FIELDKEY_VOLUME);
			}

			if ((*it).second.m_uiWeight != stQuotation.m_uiWeight)
			{
				bsQuotation.set(FIELDKEY_WEIGHT);
			}

			if ((*it).second.m_uiTurnOver != stQuotation.m_uiTurnOver)
			{
				bsQuotation.set(FIELDKEY_TURNOVER);
			}

			if ((*it).second.m_uiChiCangLiang != stQuotation.m_uiChiCangLiang)
			{
				bsQuotation.set(FIELDKEY_POSI);
			}

			if ((*it).second.m_uiLastChiCangLiang != stQuotation.m_uiLastChiCangLiang)
			{
				//
			}

			if ((*it).second.m_Bid[0].m_uiPrice != stQuotation.m_Bid[0].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_BID1);
			}

			if ((*it).second.m_Bid[0].m_uiVol != stQuotation.m_Bid[0].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_BIDLOT1);
			}

			if ((*it).second.m_Bid[1].m_uiPrice != stQuotation.m_Bid[1].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_BID2);
			}

			if ((*it).second.m_Bid[1].m_uiVol != stQuotation.m_Bid[1].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_BIDLOT2);
			}

			if ((*it).second.m_Bid[2].m_uiPrice != stQuotation.m_Bid[2].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_BID3);
			}

			if ((*it).second.m_Bid[2].m_uiVol != stQuotation.m_Bid[2].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_BIDLOT3);
			}

			if ((*it).second.m_Bid[3].m_uiPrice != stQuotation.m_Bid[3].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_BID4);
			}

			if ((*it).second.m_Bid[3].m_uiVol != stQuotation.m_Bid[3].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_BIDLOT4);
			}

			if ((*it).second.m_Bid[4].m_uiPrice != stQuotation.m_Bid[4].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_BID5);
			}

			if ((*it).second.m_Bid[4].m_uiVol != stQuotation.m_Bid[4].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_BIDLOT5);
			}


			//
			if ((*it).second.m_Ask[0].m_uiPrice != stQuotation.m_Ask[0].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_ASK1);
			}

			if ((*it).second.m_Ask[0].m_uiVol != stQuotation.m_Ask[0].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_ASKLOT1);
			}

			if ((*it).second.m_Ask[1].m_uiPrice != stQuotation.m_Ask[1].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_ASK2);
			}

			if ((*it).second.m_Ask[1].m_uiVol != stQuotation.m_Ask[1].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_ASKLOT2);
			}

			if ((*it).second.m_Ask[2].m_uiPrice != stQuotation.m_Ask[2].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_ASK3);
			}

			if ((*it).second.m_Ask[2].m_uiVol != stQuotation.m_Ask[2].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_ASKLOT3);
			}

			if ((*it).second.m_Ask[3].m_uiPrice != stQuotation.m_Ask[3].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_ASK4);
			}

			if ((*it).second.m_Ask[3].m_uiVol != stQuotation.m_Ask[3].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_ASKLOT4);
			}

			if ((*it).second.m_Ask[4].m_uiPrice != stQuotation.m_Ask[4].m_uiPrice)
			{
				bsQuotation.set(FIELDKEY_ASK5);
			}

			if ((*it).second.m_Ask[4].m_uiVol != stQuotation.m_Ask[4].m_uiVol)
			{
				bsQuotation.set(FIELDKEY_ASKLOT5);
			}

			if (bsQuotation.any())
			{
				if ((*it).second.m_uiSeqNo != stQuotation.m_uiSeqNo)
				{
					bsQuotation.set(FIELDKEY_SEQUENCENO);
				}

				if ((*it).second.m_uiTime != stQuotation.m_uiTime)
				{
					bsQuotation.set(FIELDKEY_QUOTETIME);
				}

				if ((*it).second.m_uiDate != stQuotation.m_uiDate)
				{
					bsQuotation.set(FIELDKEY_QUOTEDATE);
				}
			}

			(*it).second = stQuotation;
		}
		else
		{
			string sUnitKey = ToHexString<unsigned int>(stQuotation.m_CodeInfo.m_usMarketType,false,true,4) + "." + stQuotation.m_CodeInfo.m_acCode;
			map<string, unsigned int>::iterator it = m_mapUnit.find(sUnitKey);
			if (it != m_mapUnit.end())
			{
				stQuotation.m_CodeInfo.m_uiUnit = (*it).second;
			}
			else
			{
				sUnitKey = ToHexString<unsigned int>(stQuotation.m_CodeInfo.m_usMarketType,false,true,4) + ".*";
				it = m_mapUnit.find(sUnitKey);
				if (it != m_mapUnit.end())
				{
					stQuotation.m_CodeInfo.m_uiUnit = (*it).second;
				}
				else
				{
					stQuotation.m_CodeInfo.m_uiUnit = 1;
				}
			}
			m_mapQuotation[sKey] = stQuotation;
			bsQuotation.set();
		}
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

//发送首次订阅完整行情
int CDeliverMgr::HandleFirstQuotation(const string& sKey,unsigned int uiNodeID)
{
	try
	{
		map<string, QUOTATION>::iterator it = m_mapQuotation.find(sKey);
		if (it == m_mapQuotation.end())
		{
			return -1;
		}

		string sQuotationVal;
		QUOTATION& stQuotation = (*it).second;

		// added by Ben 20110524 若数据比当前时间旧超过30秒则不发完整包
		CGessDate oDateQuo;
		oDateQuo.FromString(ToString<unsigned int>(stQuotation.m_uiDate));

		CGessTime oTimeQuo;
		oTimeQuo.FromString(ToString<unsigned int>(stQuotation.m_uiTime/1000));
		
		if (oDateQuo.CompareNow() < 0 || oTimeQuo.IntervalToNow() > 30)
		{
			m_mapQuotation.erase(it);
			return -1;
		}
		// end





		unsigned int uiMarket = stQuotation.m_CodeInfo.m_usMarketType;
		uiMarket = htonl(uiMarket);
		sQuotationVal.append((const char*)&uiMarket,sizeof(uiMarket));

		char cPkt = 1;
		sQuotationVal.append(1,cPkt);

		bitset<FIELDKEY_UNKNOWN> bsQuotation;
		bsQuotation.set();
		AssemblePkt(stQuotation, sQuotationVal, bsQuotation);

		//
		CSamplerPacket oPkt(YL_QUOTATION);
		CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPkt.GetMsg());	

		msg.SetBinaryField(MSG_QUOTATION_RECS,sQuotationVal);
		msg.SetField(MSG_NODE_ID, (unsigned int)uiNodeID);

		if (0 != m_pProviderCpMgr)
		{
			m_pProviderCpMgr->Forward(oPkt,m_ulKey);
			m_uiOrderFwdCount++;
			m_uiOrderFwdBytes += sQuotationVal.length();
		}

		//if (Statics(sQuotationVal))
		//{
		//	CRLog(E_DEBUG,"Fwd pkts %u, %u, Bandwidth(Now-Max-Min-Avg):%u-%u-%u-%ukb",m_uiFwdCount,m_uiFwdCount,m_uiLastBandWidth,m_uiMaxBandWidth,m_uiMinBandWidth,m_uiAvgBandWidth);
		//}
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

//根据增量行情字段组报文
int CDeliverMgr::AssemblePkt(QUOTATION& stQuotation, string& sQuotationVal, bitset<FIELDKEY_UNKNOWN>& bsQuotation)
{
	try
	{
		size_t nPos = sQuotationVal.length();	
		sQuotationVal.append(1,0x00);

		//是否打包合约名称标志 全量发送需要打包名称
		unsigned char ucFlag = 0;
		if (bsQuotation.count() == bsQuotation.size())
		{
			ucFlag = 1;
		}
		sQuotationVal.append(1, ucFlag);

		//按实际长度打包合约ID
		unsigned char ucLen = CODE_LEN;
		if (stQuotation.m_CodeInfo.m_acCode[CODE_LEN-1] == '\0')
		{
			ucLen = strlen(stQuotation.m_CodeInfo.m_acCode);
		}
		sQuotationVal.append(1, ucLen);
		sQuotationVal.append(stQuotation.m_CodeInfo.m_acCode,ucLen);
		
		//根据需要按实际长度打包合约名称
		if (ucFlag == 1)
		{
			ucLen = NAME_LEN;
			if (stQuotation.m_CodeInfo.m_acName[NAME_LEN-1] == '\0')
			{
				ucLen = strlen(stQuotation.m_CodeInfo.m_acName);
			}
			sQuotationVal.append(1, ucLen);
			sQuotationVal.append(stQuotation.m_CodeInfo.m_acName,ucLen);
		}

		unsigned char cField = 0;
		unsigned int uiFieldVal = 0;
		if (bsQuotation.test(FIELDKEY_LASTSETTLE))
		{
			uiFieldVal = htonl(stQuotation.m_uiLastSettle);
			if (stQuotation.m_uiLastSettle > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_LASTSETTLE) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiLastSettle > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_LASTSETTLE) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiLastSettle > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_LASTSETTLE) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_LASTSETTLE) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}					
		}
	    
		if (bsQuotation.test(FIELDKEY_LASTCLOSE ))
		{
			uiFieldVal = htonl(stQuotation.m_uilastClose);
			if (stQuotation.m_uilastClose > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_LASTCLOSE) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uilastClose > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_LASTCLOSE) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uilastClose > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_LASTCLOSE) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_LASTCLOSE) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_OPEN      ))
		{
			uiFieldVal = htonl(stQuotation.m_uiOpenPrice);
			if (stQuotation.m_uiOpenPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_OPEN) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiOpenPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_OPEN) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiOpenPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_OPEN) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_OPEN) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_HIGH      ))
		{
			uiFieldVal = htonl(stQuotation.m_uiHigh);
			if (stQuotation.m_uiHigh > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_HIGH) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiHigh > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_HIGH) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiHigh > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_HIGH) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_HIGH) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_LOW       ))
		{
			uiFieldVal = htonl(stQuotation.m_uiLow);
			if (stQuotation.m_uiLow > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_LOW) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiLow > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_LOW) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiLow > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_LOW) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_LOW) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_LAST      ))
		{
			uiFieldVal = htonl(stQuotation.m_uiLast);
			if (stQuotation.m_uiLast > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_LAST) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiLast > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_LAST) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiLast > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_LAST) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_LAST) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_CLOSE     ))
		{
			uiFieldVal = htonl(stQuotation.m_uiClose);
			if (stQuotation.m_uiClose > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_CLOSE) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiClose > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_CLOSE) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiClose > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_CLOSE) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_CLOSE) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_SETTLE    ))
		{
			uiFieldVal = htonl(stQuotation.m_uiSettle);
			if (stQuotation.m_uiSettle > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_SETTLE) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiSettle > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_SETTLE) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiSettle > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_SETTLE) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_SETTLE) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BID1      ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[0].m_uiPrice );
			if (stQuotation.m_Bid[0].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID1) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[0].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID1) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[0].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BID1) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BID1) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BIDLOT1   ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[0].m_uiVol);
			if (stQuotation.m_Bid[0].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT1) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[0].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT1) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[0].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT1) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BIDLOT1) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BID2      ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[1].m_uiPrice);
			if (stQuotation.m_Bid[1].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID2) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[1].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID2) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[1].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BID2) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BID2) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BIDLOT2   ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[1].m_uiVol);
			if (stQuotation.m_Bid[1].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT2) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[1].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT2) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[1].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT2) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BIDLOT2) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BID3      ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[2].m_uiPrice);
			if (stQuotation.m_Bid[2].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID3) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[2].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID3) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[2].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BID3) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BID3) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BIDLOT3   ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[2].m_uiVol);
			if (stQuotation.m_Bid[2].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT3) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[2].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT3) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[2].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT3) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BIDLOT3) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BID4      ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[3].m_uiPrice);
			if (stQuotation.m_Bid[3].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID4) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[3].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID4) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[3].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BID4) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BID4) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BIDLOT4   ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[3].m_uiVol);
			if (stQuotation.m_Bid[3].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT4) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[3].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT4) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[3].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT4) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BIDLOT4) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BID5      ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[4].m_uiPrice);
			if (stQuotation.m_Bid[4].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID5) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[4].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BID5) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[4].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BID5) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BID5) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_BIDLOT5   ))
		{
			uiFieldVal = htonl(stQuotation.m_Bid[4].m_uiVol);
			if (stQuotation.m_Bid[4].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT5) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Bid[4].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT5) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Bid[4].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_BIDLOT5) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_BIDLOT5) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASK1      ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[0].m_uiPrice);
			if (stQuotation.m_Ask[0].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK1) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[0].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK1) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[0].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASK1) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASK1) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASKLOT1   ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[0].m_uiVol);
			if (stQuotation.m_Ask[0].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT1) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[0].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT1) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[0].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT1) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASKLOT1) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASK2      ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[1].m_uiPrice);
			if (stQuotation.m_Ask[1].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK2) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[1].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK2) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[1].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASK2) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASK2) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASKLOT2   ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[1].m_uiVol);
			if (stQuotation.m_Ask[1].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT2) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[1].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT2) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[1].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT2) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASKLOT2) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASK3      ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[2].m_uiPrice);
			if (stQuotation.m_Ask[2].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK3) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[2].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK3) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[2].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASK3) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASK3) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASKLOT3   ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[2].m_uiVol);
			if (stQuotation.m_Ask[2].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT3) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[2].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT3) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[2].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT3) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASKLOT3) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASK4      ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[3].m_uiPrice);
			if (stQuotation.m_Ask[3].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK4) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[3].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK4) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[3].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASK4) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASK4) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASKLOT4   ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[3].m_uiVol);
			if (stQuotation.m_Ask[3].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT4) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[3].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT4) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[3].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT4) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASKLOT4) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASK5      ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[4].m_uiPrice);
			if (stQuotation.m_Ask[4].m_uiPrice > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK5) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[4].m_uiPrice > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASK5) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[4].m_uiPrice > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASK5) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASK5) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_ASKLOT5   ))
		{
			uiFieldVal = htonl(stQuotation.m_Ask[4].m_uiVol);
			if (stQuotation.m_Ask[4].m_uiVol > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT5) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_Ask[4].m_uiVol > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT5) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_Ask[4].m_uiVol > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_ASKLOT5) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_ASKLOT5) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_VOLUME    ))
		{
			uiFieldVal = htonl(stQuotation.m_uiVolume);
			if (stQuotation.m_uiVolume > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_VOLUME) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiVolume > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_VOLUME) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiVolume > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_VOLUME) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_VOLUME) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_WEIGHT    ))
		{
			uiFieldVal = htonl(stQuotation.m_uiWeight);
			if (stQuotation.m_uiWeight > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_WEIGHT) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiWeight > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_WEIGHT) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiWeight > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_WEIGHT) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_WEIGHT) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_HIGHLIMIT ))
		{
			uiFieldVal = htonl(stQuotation.m_uiHighLimit);
			if (stQuotation.m_uiHighLimit > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_HIGHLIMIT) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiHighLimit > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_HIGHLIMIT) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiHighLimit > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_HIGHLIMIT) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_HIGHLIMIT) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_LOWLIMIT  ))
		{
			uiFieldVal = htonl(stQuotation.m_uiLowLimit);
			if (stQuotation.m_uiLowLimit > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_LOWLIMIT) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiLowLimit > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_LOWLIMIT) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiLowLimit > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_LOWLIMIT) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_LOWLIMIT) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_POSI      ))
		{
			uiFieldVal = htonl(stQuotation.m_uiChiCangLiang);
			if (stQuotation.m_uiChiCangLiang > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_POSI) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiChiCangLiang > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_POSI) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiChiCangLiang > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_POSI) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_POSI) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_UPDOWN    ))
		{
			
		}
	    
		if (bsQuotation.test(FIELDKEY_TURNOVER  ))
		{
			uiFieldVal = htonl(stQuotation.m_uiTurnOver);
			if (stQuotation.m_uiTurnOver > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_TURNOVER) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiTurnOver > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_TURNOVER) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiTurnOver > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_TURNOVER) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_TURNOVER) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_AVERAGE   ))
		{
			uiFieldVal = htonl(stQuotation.m_uiAverage);
			if (stQuotation.m_uiAverage > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_AVERAGE) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiAverage > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_AVERAGE) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiAverage > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_AVERAGE) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_AVERAGE) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_SEQUENCENO))
		{
			uiFieldVal = htonl(stQuotation.m_uiSeqNo);
			if (stQuotation.m_uiSeqNo > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_SEQUENCENO) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiSeqNo > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_SEQUENCENO) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiSeqNo > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_SEQUENCENO) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_SEQUENCENO) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_QUOTETIME ))
		{
			uiFieldVal = htonl(stQuotation.m_uiTime);
			if (stQuotation.m_uiTime > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_QUOTETIME) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiTime > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_QUOTETIME) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiTime > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_QUOTETIME) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_QUOTETIME) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}
	    
		if (bsQuotation.test(FIELDKEY_UPDOWNRATE))
		{

		}
	    
		if (bsQuotation.test(FIELDKEY_QUOTEDATE ))
		{
			uiFieldVal = htonl(stQuotation.m_uiDate);
			if (stQuotation.m_uiDate > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_QUOTEDATE) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_uiDate > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_QUOTEDATE) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_uiDate > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_QUOTEDATE) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_QUOTEDATE) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}		
		}

		if (bsQuotation.test(FIELDKEY_UNIT ))
		{
			uiFieldVal = htonl(stQuotation.m_CodeInfo.m_uiUnit);
			if (stQuotation.m_CodeInfo.m_uiUnit > 0xFFFFFF)
			{
				cField = ((0xFF & FIELDKEY_UNIT) << 2) | 0x03;
				sQuotationVal.append(1,cField);						
				sQuotationVal.append((const char*)&uiFieldVal,4);
			}
			else if (stQuotation.m_CodeInfo.m_uiUnit > 0xFFFF)
			{
				cField = ((0xFF & FIELDKEY_UNIT) << 2) | 0x02;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+1,3);
			}
			else if (stQuotation.m_CodeInfo.m_uiUnit > 0xFF)
			{
				cField = ((0xFF & FIELDKEY_UNIT) << 2) | 0x01;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+2,2);
			}
			else
			{
				cField = ((0xFF & FIELDKEY_UNIT) << 2) | 0x00;
				sQuotationVal.append(1,cField);
				
				sQuotationVal.append(((const char*)&uiFieldVal)+3,1);
			}
		}

		size_t nLen = sQuotationVal.length() - nPos - 1;
		unsigned char cLen = 0;
		if (nLen > 0xFF)
		{
			CRLog(E_ERROR,"pkt len error");
			sQuotationVal.clear();
		}
		else
		{
			cLen = static_cast<unsigned char>(nLen);
			string sTmp;
			sTmp.append(1,cLen);
			sQuotationVal.replace(nPos,1,sTmp);
		}
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

//根据订阅发布
int CDeliverMgr::DelieveQuotation(unsigned int uiMarketType, string& sQuotationVal)
{
	try
	{
		CSamplerPacket oPktDest(YL_QUOTATION);
		CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPktDest.GetMsg());	
		msg.SetBinaryField(MSG_QUOTATION_RECS,sQuotationVal);

		unsigned int uiCount = 0;
		map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();
		if( !mapSubscri.empty())
		{
			map<unsigned int,SUB_CONTEXT>::iterator it;
			for(it = mapSubscri.begin();it != mapSubscri.end();++it)
			{
				if(0 != CheckSendMsgRole(it->second, uiMarketType))
				{					
					continue;
				}
				msg.SetField(MSG_NODE_ID,it->second.nodeId);

				if (0 != m_pProviderCpMgr)
				{
					int nRtn = m_pProviderCpMgr->Forward(oPktDest,m_ulKey);
					if (0 > nRtn && nRtn != -2)
					{//检查到已经异常中断或未取消订阅的接口 
						CMemData::Instance()->GetSubscriberTbl().CancelSubscriber(it->second.nodeId);
					}
					else
					{
						uiCount++;
					}
				}
			}
		}

		if (Statics(sQuotationVal))
		{
			double dlQuoPerPkt = 0.0;
			if (m_uiFwdCount != 0)
			{
				dlQuoPerPkt = (m_uiQuoPktCount*1.0)/m_uiFwdCount;
			}

			unsigned int uiBytesPerPkt = 0;
			if (m_uiQuoPktCount != 0)
			{
				uiBytesPerPkt = (m_uiQuoPktGBytes*GBYTES+m_uiQuoPktBytes)/m_uiQuoPktCount;
			}
			CRLog(E_STATICS,"Fwd pkts %u(%u), %uM(%0.1fQ/pkt,%uB/pkt), Bandwidth(Now-Max-Min-Avg):%u-%u-%u-%ukb",m_uiFwdCount, uiCount,m_uiQuoPktGBytes*1024+m_uiQuoPktBytes/MBYTES,dlQuoPerPkt,uiBytesPerPkt,m_uiLastBandWidth,m_uiMaxBandWidth,m_uiMinBandWidth,m_uiAvgBandWidth);
		}
		
		if (0 != m_pProviderCpMgr)
		{
			m_pProviderCpMgr->ToHisData(sQuotationVal);
		}
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

//流量统计
bool CDeliverMgr::Statics(string& sQuotationVal)
{
	try
	{
		bool blLog = false;

		m_uiFwdCount++;
		unsigned int uiTmpCount = m_uiFwdCount;	
		m_uiFwdBytes += (sQuotationVal.length() + 2 + sizeof(SAMPLER_PKT_HEADER) + 28);
		if (m_uiFwdBytes >= GBYTES)
		{
			m_uiFwdGBytes++;
			m_uiFwdBytes -= GBYTES;
		}

		int nInterval = m_oRecTime.IntervalToNow();
		if ((m_uiFwdCount - 1) % 5000 == 0 || nInterval > 300)
		{
			int nIntervalDay = m_oRecDate.IntervalToToday();
			if (nIntervalDay > 0)
			{
				m_oRecDate.Day2Today();
				m_oRecTime.ToNow();
				m_uiFwdCount = 0;
				m_uiFwdGBytes = 0;
				m_uiFwdBytes = 0;
				m_uiFwdGBytesLast = 0;
				m_uiFwdBytesLast = 0;
				m_nInterval = 0;
				m_uiMaxBandWidth = 0;
				m_uiMinBandWidth = 0;
				m_uiAvgBandWidth = 0;
				m_uiQuoPktCount = 0;
				m_uiQuoPktBytes = 0;
				m_uiQuoPktGBytes = 0;
				m_uiOrderFwdCount = 0;
				m_uiOrderFwdBytes = 0;
			}
			else
			{				
				if (nInterval >= 10)
				{
					unsigned int uiTmp = (m_uiFwdGBytes - m_uiFwdGBytesLast) * 1024 * 1024 / nInterval * 8  + (m_uiFwdBytes - m_uiFwdBytesLast) / 1024 * 8 / nInterval;
					m_uiLastBandWidth = uiTmp;
					if (m_uiMaxBandWidth < uiTmp)
						m_uiMaxBandWidth = uiTmp;
					if (m_uiMinBandWidth == 0 || m_uiMinBandWidth > uiTmp)
						m_uiMinBandWidth = uiTmp;

					m_nInterval += nInterval;
					m_uiAvgBandWidth = 1024 * 1024 * 8 / m_nInterval * m_uiFwdGBytes + m_uiFwdBytes / 1024 * 8 / m_nInterval;
					
					m_uiFwdGBytesLast = m_uiFwdGBytes;
					m_uiFwdBytesLast = m_uiFwdBytes;
					m_oRecTime.ToNow();
					
					blLog = true;
					
				}
			}
		}
		return blLog;
	}
	catch(std::exception e)
	{
		CRLog(E_ERROR,"exception:%s!",e.what());
		return false;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return false;
	}
}

///根据结点订阅信息判断当前合约是否需要通知，0--需要通知，非0-无需通知
int CDeliverMgr::CheckSendMsgRole(SUB_CONTEXT& SubscriberContext,const unsigned int & musMarketType)
{

	try
	{

		//包括指定的列表信息
		if( '0' == SubscriberContext.subType)
		{
			vector<unsigned int>::iterator itVec=SubscriberContext.vecInstId.begin();
			for( ;itVec!= SubscriberContext.vecInstId.end();++itVec)
			{
				if( (*itVec) == musMarketType || (*itVec) == 0xFFFFFFFF)
				{
					return 0;
				}
			}
			return 1;
		}

		//除列表中列表中的项除外
		else if( '1' == SubscriberContext.subType)
		{
			vector<unsigned int>::iterator itVec=SubscriberContext.vecInstId.begin();
			for( ;itVec!= SubscriberContext.vecInstId.end();++itVec)
			{
				if( (*itVec) == musMarketType)
				{
					return 1;
				}
			}
			return 0;
		}else
		{
			return -1;
		}	
	}
	catch (std::exception e)
	{
		CRLog(E_ERROR, "::CheckSendMsgRole exception:%s",e.what()); 
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"::CheckSendMsgRole Unknown exception");
		return -1;
	}
}

int CDeliverMgr::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","CDeliverMgr thread wait end");
	Wait();
	return 0;
}

//命令行命令处理
string CDeliverMgr::HandleCmdLine(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Provider->\r\n";
	
	if (sCmd == "r" || sCmd == "replay")
	{
		return HandleCmdLineReplay(sCmd, vecPara);
	}
	else if (sCmd == "b" || sCmd == "buf")
	{
		return OnCmdLineBuffer(sCmd, vecPara);
	}
	else if (sCmd == "ld" || sCmd == "load")
	{
		return OnCmdLineLoad(sCmd, vecPara);
	}
	else
	{
		return sRtn;
	}
}

//重新加载合约每手单位配置文件
string CDeliverMgr::OnCmdLineLoad(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn = "Provider->\r\n";

	try
	{
		m_deqCondMutex.Lock();

		if (vecPara.size() == 0)
		{
			m_mapUnit.clear();
			m_oUnitFile.Clear();

			std::string sUnitFile;
			sUnitFile = DEFUALT_CONF_PATH PATH_SLASH;
			sUnitFile = sUnitFile + "Unit";
			sUnitFile = sUnitFile + ".cfg";
			
			m_oUnitFile.Load(sUnitFile);
			vector<string> vUnit = m_oUnitFile.GetKeys();
			for (vector<string>::iterator it = vUnit.begin(); it != vUnit.end(); ++it)
			{
				string sTmp;
				if (0 == m_oUnitFile.GetProperty(*it, sTmp))
				{
					unsigned int uiVal = FromString<unsigned int>(sTmp);
					if (uiVal == 0)
						uiVal = 1;

					m_mapUnit[*it] = uiVal;
				}
			}

			for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
			{
				bitset<FIELDKEY_UNKNOWN> bsQuotation;
				bsQuotation.set(FIELDKEY_UNIT);

				string sQuotationVal;
				unsigned int uiMarket = (*it).second.m_CodeInfo.m_usMarketType;
				unsigned int uiTmp = htonl(uiMarket);
				unsigned char cPkt = 1;
				sQuotationVal.append((const char*)&uiTmp,sizeof(uiTmp));
				sQuotationVal.append(1,cPkt);

				AssemblePkt((*it).second, sQuotationVal, bsQuotation);
				DelieveQuotation(uiMarket, sQuotationVal);
			}

			sRtn = "Load ok!\r\n";
			sRtn += "Provider->\r\n";
		}
		else
		{
			if (vecPara[0] == "show")
			{
				for (map<string, unsigned int>::iterator it = m_mapUnit.begin(); it != m_mapUnit.end(); ++it)
				{
					sRtn += (*it).first + " = " + ToString<unsigned int>((*it).second);
					sRtn += "\r\n";
				}
			}
		}

		m_deqCondMutex.Unlock();
		return sRtn;
	}
	catch(...)
	{
		m_deqCondMutex.Unlock();
		return sRtn;
	}
}

//缓存行情显示
string CDeliverMgr::OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara)
{
	string sRtn;
	string sInstID;

	if (vecPara.size() >  0)
	{
		sInstID = vecPara[0];
	}

	try
	{
		m_deqCondMutex.Lock();
		if (sInstID.empty())
		{
			for (map<string,QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
			{
				sRtn += ToHexString<unsigned int>((*it).second.m_CodeInfo.m_usMarketType);
				sRtn += "\t";
				sRtn += (*it).second.m_CodeInfo.m_acCode;
				sRtn += "\t";
				sRtn += (*it).second.m_CodeInfo.m_acName;
				sRtn += "\t";
				sRtn += ToString<unsigned int>((*it).second.m_CodeInfo.m_uiUnit);
				sRtn += "\r\n";
			}
			sRtn += "Total ";
			sRtn += ToString<unsigned int>(m_mapQuotation.size());
			sRtn += "\r\n";
		}
		else
		{
			for (map<string,QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
			{
				string sTmpID = (*it).second.m_CodeInfo.m_acCode;
				if (sTmpID == sInstID)
				{
					sRtn += ToHexString<unsigned int>((*it).second.m_CodeInfo.m_usMarketType);
					sRtn += "\t";
					sRtn += (*it).second.m_CodeInfo.m_acCode;
					sRtn += "\t";
					sRtn += (*it).second.m_CodeInfo.m_acName;
					sRtn += "\t";
					sRtn += ToString<unsigned int>((*it).second.m_CodeInfo.m_uiUnit);
					sRtn += "\r\n";
					
					sRtn += "m_uiSeqNo:";        
					sRtn += ToString<unsigned int>((*it).second.m_uiSeqNo);
					sRtn += "\r\n";
					sRtn += "m_uiDate:";            
					sRtn += ToString<unsigned int>((*it).second.m_uiDate);
					sRtn += "\r\n";
					sRtn += "m_uiTime:";            
					sRtn += ToString<unsigned int>((*it).second.m_uiTime);
					sRtn += "\r\n";
					sRtn += "m_uilastClose:";    
					sRtn += ToString<unsigned int>((*it).second.m_uilastClose);
					sRtn += "\r\n";
					sRtn += "m_uiLastSettle:"; 
					sRtn += ToString<unsigned int>((*it).second.m_uiLastSettle);
					sRtn += "\r\n";
					sRtn += "m_uiSettle:";   
					sRtn += ToString<unsigned int>((*it).second.m_uiSettle);     
					sRtn += "\r\n";
					sRtn += "m_uiOpenPrice:";    
					sRtn += ToString<unsigned int>((*it).second.m_uiOpenPrice);
					sRtn += "\r\n";
					sRtn += "m_uiHigh:";            
					sRtn += ToString<unsigned int>((*it).second.m_uiHigh);
					sRtn += "\r\n";
					sRtn += "m_uiLow:";             
					sRtn += ToString<unsigned int>((*it).second.m_uiLow);
					sRtn += "\r\n";
					sRtn += "m_uiClose:";           
					sRtn += ToString<unsigned int>((*it).second.m_uiClose);
					sRtn += "\r\n";
					sRtn += "m_uiHighLimit:";       
					sRtn += ToString<unsigned int>((*it).second.m_uiHighLimit);
					sRtn += "\r\n";
					sRtn += "m_uiLowLimit:";	       
					sRtn += ToString<unsigned int>((*it).second.m_uiLowLimit);
					sRtn += "\r\n";
					sRtn += "m_uiLast:";            
					sRtn += ToString<unsigned int>((*it).second.m_uiLast);
					sRtn += "\r\n";
					sRtn += "m_uiAverage:";        
					sRtn += ToString<unsigned int>((*it).second.m_uiAverage);
					sRtn += "\r\n";
					sRtn += "m_uiVolume:";          
					sRtn += ToString<unsigned int>((*it).second.m_uiVolume);
					sRtn += "\r\n";
					sRtn += "m_uiTurnOver:";        
					sRtn += ToString<unsigned int>((*it).second.m_uiTurnOver);
					sRtn += "\r\n";
					sRtn += "m_uiChiCangLiang:";    
					sRtn += ToString<unsigned int>((*it).second.m_uiChiCangLiang);
					sRtn += "\r\n";
					sRtn += "m_uiLastChiCangLiang:";
					sRtn += ToString<unsigned int>((*it).second.m_uiLastChiCangLiang);
					for (int nIndex = 0; nIndex < 5; nIndex++)
					{
						sRtn += "\r\n";
						sRtn += "Bid" + ToString<int>(nIndex+1) + ":";
						sRtn += ToString<unsigned int>((*it).second.m_Bid[nIndex].m_uiPrice);
						sRtn += "\t";
						sRtn += "Bidlot" + ToString<int>(nIndex+1) + ":";
						sRtn += ToString<unsigned int>((*it).second.m_Bid[nIndex].m_uiVol);

						sRtn += "\r\n";
						sRtn += "Ask" + ToString<int>(nIndex+1) + ":";
						sRtn += ToString<unsigned int>((*it).second.m_Ask[nIndex].m_uiPrice);
						sRtn += "\t";
						sRtn += "Asklot" + ToString<int>(nIndex+1) + ":";
						sRtn += ToString<unsigned int>((*it).second.m_Ask[nIndex].m_uiVol);
					}
					sRtn += "\r\n";
					sRtn += "\r\n";
				}
			}
		}
		m_deqCondMutex.Unlock();
	}
	catch(...)
	{
		sRtn = "Cvg->";
		m_deqCondMutex.Unlock();
	}
	return sRtn;
}

//重放命令
string CDeliverMgr::HandleCmdLineReplay(const string& sCmd, const vector<string>& vecPara)
{
	//////////////////////////////////
	////临时转换使用
	//string sInFullPath = ".\\hisdata\\20100927090407.dat";
	//ifstream ifDataTranslator;
	//ifDataTranslator.open(sInFullPath.c_str(),ios::in | ios::binary);
	//if (ifDataTranslator.fail())
	//{
	//	string ssRtn = "open file fail!\r\n";
	//	return ssRtn;
	//}

	//ofstream ofsQuotation;
	//string sOutFileName = CGessDate::NowToString("");
	//sOutFileName += CGessTime::NowToString();
	//sOutFileName += ".dat";
	//sOutFileName = ".\\hisdata\\" + sOutFileName;
	//ofsQuotation.open(sOutFileName.c_str(),ios::binary | ios::out | ios::trunc);

	//map<string,string> mapKey;
	//
	//do
	//{
	//	string sQuotation;
	//	unsigned char szBuf[5] = {0};
	//	
	//	ifDataTranslator.read((char *)szBuf,sizeof(szBuf));
	//	if (sizeof(szBuf) > ifDataTranslator.gcount() || ifDataTranslator.eof())
	//		break;

	//	unsigned char ucPkts = szBuf[4];
	//	sQuotation.assign((const char*)szBuf,sizeof(szBuf));

	//	string sQuoTmp;
	//	for (unsigned char ucIndex = 1; ucIndex <= ucPkts; ucIndex++)
	//	{
	//		unsigned char ucSize = 0;
	//		ifDataTranslator.read((char *)(&ucSize),sizeof(ucSize));
	//		if (ifDataTranslator.eof())
	//			break;

	//		if (ucSize == 0)
	//			continue;

	//		unsigned char szPkt[255] = {0};
	//		ifDataTranslator.read((char *)(szPkt),ucSize);
	//		if (ifDataTranslator.eof())
	//			break;

	//		char szCode[CODE_LEN] = {0};
	//		memcpy(szCode, szPkt, 8);
	//		sQuoTmp.append(szCode, CODE_LEN);

	//		char szName[NAME_LEN] = {0};
	//		memcpy(szName, szPkt, 8);
	//		sQuoTmp.append(szName, NAME_LEN);

	//		sQuoTmp.append((char*)(&szPkt[8]), ucSize-8);

	//		ucSize = sQuoTmp.length();
	//		sQuotation.append((char*)(&ucSize), 1);
	//		sQuotation.append(sQuoTmp.data(), sQuoTmp.length());
	//		sQuoTmp.clear();
	//	}

	//	ofsQuotation.write(sQuotation.data(), sQuotation.length());
	//	ofsQuotation.flush();
	//} while (!ifDataTranslator.eof());

	//ifDataTranslator.close();
	//ofsQuotation.close();
	//return "";
	//////////////////////////////////


	//////////////////////////////////
	//string sInFullPath = ".\\hisdata\\20100927090407.dat";
	//ifstream ifDataTranslator;
	//ifDataTranslator.open(sInFullPath.c_str(),ios::in | ios::binary);
	//if (ifDataTranslator.fail())
	//{
	//	string ssRtn = "open file fail!\r\n";
	//	return ssRtn;
	//}

	//ofstream ofsQuotation;
	//string sOutFileName = CGessDate::NowToString("");
	//sOutFileName += CGessTime::NowToString();
	//sOutFileName += ".dat";
	//sOutFileName = ".\\hisdata\\" + sOutFileName;
	//ofsQuotation.open(sOutFileName.c_str(),ios::binary | ios::out | ios::trunc);

	//map<string,string> mapKey;
	//
	//do
	//{
	//	string sQuotation;
	//	unsigned char szBuf[5] = {0};
	//	
	//	ifDataTranslator.read((char *)szBuf,sizeof(szBuf));
	//	if (sizeof(szBuf) > ifDataTranslator.gcount() || ifDataTranslator.eof())
	//		break;

	//	unsigned char ucPkts = szBuf[4];
	//	sQuotation.assign((const char*)szBuf,sizeof(szBuf));

	//	for (unsigned char ucIndex = 1; ucIndex <= ucPkts; ucIndex++)
	//	{
	//		string sQuoTmp;

	//		unsigned char ucSize = 0;
	//		ifDataTranslator.read((char *)(&ucSize),sizeof(ucSize));
	//		if (ifDataTranslator.eof())
	//			break;

	//		if (ucSize == 0)
	//			continue;

	//		unsigned char szPkt[255] = {0};
	//		ifDataTranslator.read((char *)(szPkt),ucSize);
	//		if (ifDataTranslator.eof())
	//			break;

	//		char szCode[8] = {0};
	//		memcpy(szCode, szPkt, 8);
	//		unsigned char ucLen = CODE_LEN;
	//		if (szCode[7] == '\0')
	//		{
	//			ucLen = strlen(szCode);
	//		}
	//		
	//		unsigned ucFlag = 0;
	//		string sKey;
	//		sKey.assign(szCode, sizeof(szCode));
	//		map<string,string>::iterator it = mapKey.find(sKey);
	//		if (it == mapKey.end())
	//		{
	//			ucFlag = 1;
	//			mapKey[sKey] = sKey;
	//		}
	//		else
	//		{
	//			ucFlag = 0;
	//		}

	//		sQuoTmp.append((char*)(&ucFlag), 1);
	//		sQuoTmp.append((char*)(&ucLen),1);
	//		sQuoTmp.append(szCode, ucLen);
	//		if (ucFlag == 1)
	//		{
	//			sQuoTmp.append((char*)(&ucLen),1);
	//			sQuoTmp.append(szCode, ucLen);
	//		}
	//		sQuoTmp.append((char*)(&szPkt[8]), ucSize-8);

	//		ucSize = sQuoTmp.length();
	//		sQuotation.append((char*)(&ucSize), 1);
	//		sQuotation.append(sQuoTmp.data(), sQuoTmp.length());
	//		sQuoTmp.clear();
	//	}

	//	ofsQuotation.write(sQuotation.data(), sQuotation.length());
	//	ofsQuotation.flush();
	//} while (!ifDataTranslator.eof());

	//ifDataTranslator.close();
	//ofsQuotation.close();
	//return "";
	//////////////////////////////////


	string sRtn = "Provider->\r\n";

	string sMarket = "*";
	string sInstID = "*";
	string sInterval = "*";
	string sCount = "*";
	string sFileName = "*";
	if (vecPara.size() > 0)
	{
		sFileName = vecPara[0];

		if (vecPara.size() > 1)
		{
			sMarket = vecPara[1];

			if (vecPara.size() > 2)
			{
				sInstID = vecPara[2];							

				if (vecPara.size() > 3)
				{
					sInterval = vecPara[3];	
									
					if (vecPara.size() > 4)
					{
						sCount = vecPara[4];
					}
				}
			}			
		}
	}

	string sFullPath = ".\\hisdata\\" + sFileName;
	ifstream ifData;
	ifData.open(sFullPath.c_str(),ios::in | ios::binary);
	if (ifData.fail())
	{
		sRtn += "open file fail!\r\n";
		return sRtn;
	}

	unsigned int uiParaMarket = 0xFFFFFFFF;
	if (sMarket != "*")
	{
		uiParaMarket = FromString<unsigned int>(sMarket, 16);
	}

	unsigned int nInterval = 0;
	if (sInterval != "*")
	{
		nInterval = FromString<unsigned int>(sInterval);
		if (nInterval > 5000)// || nInterval < 10)
			nInterval = 1000;
	}

	unsigned int nPlanCount = 0xFFFFFFFF;
	if (sCount != "*")
	{
		nPlanCount = FromString<int>(sCount);
	}

	//
	for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
	{
		unsigned int uiMarket = (*it).second.m_CodeInfo.m_usMarketType;
		if (uiParaMarket != 0xFFFFFFFF && ((uiParaMarket & 0x00000FFF) != 0 && uiMarket != uiParaMarket) || ((uiParaMarket & 0x00000FFF) == 0 && (uiMarket & uiParaMarket) != uiParaMarket) )
			continue;

		if (sInstID != "*")
		{
			char szCode[CODE_LEN] = {0};
			memcpy(szCode, (*it).second.m_CodeInfo.m_acCode, sizeof(szCode));
			string sTmp;
			sTmp.assign(szCode, sizeof(szCode));
			if (0 != sTmp.compare(0, min(sTmp.length(), sInstID.length()), sInstID))
				continue;
		}

		bitset<FIELDKEY_UNKNOWN> bsQuotation;
		bsQuotation.set();
		string sQuotationVal;
		unsigned int uiTmp = htonl(uiMarket);
		unsigned char cPkt = 1;
		sQuotationVal.append((const char*)&uiTmp,sizeof(uiTmp));
		sQuotationVal.append(1,cPkt);

		AssemblePkt((*it).second, sQuotationVal, bsQuotation);
		DelieveQuotation(uiMarket, sQuotationVal);
	}

	unsigned int nCount = 0;
	do
	{
		bool blFlag = true;

		string sQuotation;
		unsigned char szBuf[5] = {0};
		
		ifData.read((char *)szBuf,sizeof(szBuf));
		if (sizeof(szBuf) > ifData.gcount() || ifData.eof())
			break;

		unsigned int uiMarket = *(unsigned int*)(szBuf);
		uiMarket = ntohl(uiMarket);
		if (uiParaMarket != 0xFFFFFFFF && ((uiParaMarket & 0x00000FFF) != 0 && uiMarket != uiParaMarket) || ((uiParaMarket & 0x00000FFF) == 0 && (uiMarket & uiParaMarket) != uiParaMarket) )
			blFlag = false;

		unsigned char ucPkts = szBuf[4];
		sQuotation.assign((const char*)szBuf,sizeof(szBuf));

		unsigned char ucInstCount = 0;
		for (unsigned char ucIndex = 1; ucIndex <= ucPkts; ucIndex++)
		{
			bool blFlagInst = true;

			unsigned char ucSize = 0;
			ifData.read((char *)(&ucSize),sizeof(ucSize));
			if (ifData.eof())
				break;

			if (ucSize == 0)
				continue;

			unsigned char szPkt[255] = {0};
			ifData.read((char *)(szPkt),ucSize);
			if (ifData.eof())
				break;

			if (sInstID != "*")
			{
				char szCode[CODE_LEN] = {0};
				unsigned char ucLen = szPkt[1];
				memcpy(szCode, &szPkt[2], ucLen);
				string sTmp;
				sTmp.assign(szCode, ucLen);
				if (0 != sTmp.compare(0, min(sTmp.length(), sInstID.length()), sInstID))
				{
					blFlagInst = false;
				}
			}

			if (blFlagInst)
			{
				sQuotation.append(1, ucSize);
				sQuotation.append((const char*)szPkt, ucSize);

				ucInstCount++;
			}
		}

		if (blFlag && ucInstCount > 0)
		{
			if (ucInstCount != ucPkts)
			{
				string sTmp;
				sTmp.append(1,ucInstCount);
				sQuotation.replace(sizeof(unsigned int),1,sTmp);
			}

			DelieveQuotation(uiMarket, sQuotation);

			nCount++;
			if (nCount >= nPlanCount)
				break;

			if (nInterval > 0)
				Sleep(nInterval);
		}
	} while (!ifData.eof());

	ifData.close();
	
	sRtn += "重放:";
	sRtn += ToString<int>(nCount);
	sRtn += "条报文！";
	return sRtn;
}
