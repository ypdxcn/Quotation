#include "DeliverMgr.h"
#include "QuoSvrCpMgr.h"
#include "MibConstant.h"
#include "Logger.h"
#include "strutils.h"

using namespace MibConst;

#define GBYTES	(1024 * 1024 * 1024)
#define MBYTES	(1024 * 1024)


CDeliverMgr::CDeliverMgr(void)
:m_pQuoSvrCpMgr(0)
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
,m_uiQuoPktCount(0)
,m_uiQuoPktBytes(0)
,m_uiQuoPktGBytes(0)
,m_uiOrderFwdCount(0)
,m_uiOrderFwdBytes(0)
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

    // 增加读取过滤设置, Jerry Lee, 2012-3-22
    // begin
    // 星期一的21:00至23:59点
    TimeSlice ts;
    ts.begin.hour = 21;
    ts.begin.minute = 0;
    ts.end.hour = 23;
    ts.end.minute = 59;
    m_timeFilters[6].slices.push_back(ts);

    // 星期天的00:00至05:59点
    ts.begin.hour = 0;
    ts.begin.minute = 0;
    ts.end.hour = 5;
    ts.end.minute = 59;
    m_timeFilters[0].slices.push_back(ts);


    char buf[13] = {0};
    string sTmp;
    for (int i = 0; i < 7; i++)
    {   
        sprintf(buf, "FILTER.week%d", i+1);
        if (0 == m_pCfg->GetProperty(buf,sTmp))
        {
            m_timeFilters[i].set(sTmp);
        }
    }
    // end
    
	m_oMgrModule.Bind(this);
	CNetMgr::Instance()->Register(&m_oMgrModule,vpaOid);
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
	m_mmapQuotationOrder.clear();
}

void CDeliverMgr::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pQuoSvrCpMgr = dynamic_cast<CQuoSvrCpMgr*>(pCpMgr);
}


int CDeliverMgr::SendPacket(CPacket &pkt)
{
	try
	{
		CSamplerPacket & pktSrc = dynamic_cast<CSamplerPacket&>(pkt);
	
		m_deqCondMutex.Lock();
		m_deqQuotation.push_back(pktSrc);
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

int CDeliverMgr::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqQuotation.empty() && m_mmapQuotationOrder.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}
			
			if (!m_deqQuotation.empty())
			{
				CSamplerPacket& oPkt = m_deqQuotation.front();
				m_deqCondMutex.Unlock();

				HandlePacket(oPkt);

				m_deqCondMutex.Lock();
				m_deqQuotation.pop_front();
				m_deqCondMutex.Unlock();
			}
			else
			{
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

//只处理行情/订阅/取消订阅报文
int CDeliverMgr::HandlePacket(CSamplerPacket& oPkt)
{
	try
	{
		string sCmdID = oPkt.GetCmdID();
		if (strutils::ToHexString<unsigned int>(YL_QUOTATION) == sCmdID)
		{
			HandleQuotationDlv(oPkt);
		}
		else if (strutils::ToHexString<unsigned int>(YL_SUBSCRIP) == sCmdID)
		{
			HandleOrder(oPkt);
		}
		else if (strutils::ToHexString<unsigned int>(YL_UNSUBSCRIP) == sCmdID)
		{
			HandleCancelOrder(oPkt);
		}
		else
		{
			
		}

		// added by Jerry Lee, 2010-12-24, 处理历史数据报文
		unsigned int ulCmdID = strutils::FromString<unsigned int>(sCmdID);
		if (YL_HISTORYDATA == ulCmdID || YL_TICKDATA == ulCmdID || YL_INFODATA == ulCmdID)
		{
			HandleHistoryData(oPkt);
		}
		// end add

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

		MMAP_ORDER mmapOrder;
		CMemData::Instance()->GetQuotationTbl().GetInstKeySet(cSubscripType, vecMarkeType, uiNodeID, mmapOrder);
		m_mmapQuotationOrder.insert(mmapOrder.begin(),mmapOrder.end());

		CRLog(E_PROINFO,"OnRecvSubscrip NodeID:%u,mode:%d Num:%u,%s",uiNodeID, cSubscripType, mmapOrder.size(), sLog.c_str());

		///订阅时增加订阅请求
		CMemData::Instance()->GetSubscriberTbl().AddSubscripItem(uiNodeID,cSubscripType,vecMarkeType);

		CMessageImpl oMsgRsp;
		oMsgRsp.SetField(MSG_SEQ_ID,uiSeqNo);
		oMsgRsp.SetField(MSG_NODE_ID,uiNodeID);

		unsigned int nRst = 0;
		oMsgRsp.SetField(MSG_SUBSCRIP_RESULT,nRst);

		CSamplerPacket oPktRsp(oMsgRsp,YL_SUBSCRIP_RSP);
		if (0 != m_pQuoSvrCpMgr)
			return m_pQuoSvrCpMgr->Forward(oPktRsp,m_ulKey);

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
		if (0 != m_pQuoSvrCpMgr)
			return m_pQuoSvrCpMgr->Forward(oPktRsp,m_ulKey);

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

//获取报文内合约ID集合
int CDeliverMgr::GetInstID(CSamplerPacket& oPkt, map<string,string>& mapInstID)
{
	try
	{
		CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPkt.GetMsg());
		string sQuotationData;
		if (0 != msg.GetBinaryField(MSG_QUOTATION_RECS,sQuotationData))
		{
			CRLog(E_APPINFO,"读取数据错误!");
			return -1;
		}

		const unsigned char* pData = (const unsigned char*)sQuotationData.data();
		unsigned int uiMarketType = *(unsigned int*)(pData);
		uiMarketType = ntohl(uiMarketType);
		pData += sizeof(unsigned int);

		unsigned char cPkts = *pData;
		pData++;

		for (unsigned char i = 0; i < cPkts; i++)
		{
			char szCode[CODE_LEN] = {0};
			unsigned char ucPktSize = *pData;
			pData++;

			unsigned char ucLen = *(pData+1);
			memcpy(szCode,pData+2,ucLen);
			pData += ucPktSize;

			string sKey;
			sKey.assign((const char*)(&uiMarketType), sizeof(uiMarketType));
			sKey.append(szCode, sizeof(szCode));
			mapInstID[sKey] = sKey;
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

//发布实时行情
int CDeliverMgr::HandleQuotationDlv(CSamplerPacket& oPkt)
{
	try
	{
		if (!m_mmapQuotationOrder.empty())
		{//需要发送完整行情字段的第一次订阅表里 若存在当前合约，首先发送此合约完整行情字段给订阅者
		 //此表只在有新客户端订阅时才会载入数据，发送完成后清除
			map<string,string> mapKey;
			if (0 == GetInstID(oPkt, mapKey))
			{
				for (map<string,string>::iterator itmInst = mapKey.begin(); itmInst != mapKey.end(); ++itmInst)
				{
					string sKey = (*itmInst).first;
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
			}
			mapKey.clear();
		}
		
		CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPkt.GetMsg());

		string sQuotationData;
		if (0 != msg.GetBinaryField(MSG_QUOTATION_RECS,sQuotationData))
		{
			CRLog(E_APPINFO,"读取数据错误!");
			return -1;
		}

		const char* pData = sQuotationData.data();
		unsigned int uiMarketType = *(unsigned int*)(pData);
		uiMarketType = ntohl(uiMarketType);
		if (uiMarketType == 0)
		{
			CRLog(E_ERROR,"MarketType err!");
			return -1;
		}


        // 临时版本，在星期天晚上九点至星期一两点半之间的数据不转发, Jerry Lee, 2012-3-2
        SYSTEMTIME stNow;
        GetLocalTime(&stNow);
        int i = (stNow.wDayOfWeek+6)%7;

        if (m_timeFilters[i].isFilter(stNow.wHour, stNow.wMinute))
        {
            CRLog(E_APPINFO,"抛弃非交易时间段行情数据, 市场类型为: %d!", uiMarketType);

            return -1;
        }
        /*
        if ((stNow.wDayOfWeek == 0 && stNow.wHour >= 21 && stNow.wHour <= 23)
            || (stNow.wDayOfWeek == 1 && stNow.wHour >= 0 && stNow.wHour < 6))
        {
            CRLog(E_APPINFO,"抛弃非交易时间段行情数据, 市场类型为: %d!", uiMarketType);

            return -1;
        }
        */
        //

		unsigned char cPkts = *(unsigned char*)(pData + sizeof(unsigned int));
		m_uiQuoPktCount += cPkts;
		m_uiQuoPktBytes += sQuotationData.length() - sizeof(SAMPLER_PKT_HEADER) - 2 - 1 - cPkts;
		if (m_uiQuoPktBytes >= GBYTES)
		{
			m_uiQuoPktGBytes++;
			m_uiQuoPktBytes -= GBYTES;
		}

		//按订阅市场类型,NodeID分发
		unsigned int uiTmpCount = 0;
		map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();
		if( !mapSubscri.empty())
		{
			for(map<unsigned int,SUB_CONTEXT>::iterator it = mapSubscri.begin();it != mapSubscri.end();++it)
			{
				if(0 != CheckSendMsgRole(it->second,uiMarketType))
				{
					continue;
				}
				msg.SetField(MSG_NODE_ID,it->second.nodeId);

				if (0 != m_pQuoSvrCpMgr)
				{
					int nRtn = m_pQuoSvrCpMgr->Forward(oPkt,m_ulKey);
					if (0 > nRtn && nRtn != -2)
					{//检查到已经异常中断或未取消订阅的接口 
						CMemData::Instance()->GetSubscriberTbl().CancelSubscriber(it->second.nodeId);				
					}
					else
					{
						uiTmpCount++;
					}
				}
			}
		}

		if (Statics(sQuotationData))
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
			CRLog(E_STATICS,"Fwd pkts %u(%u), %uM(%0.1fQ/pkt,%uB/pkt), Bandwidth(Now-Max-Min-Avg):%u-%u-%u-%ukb",m_uiFwdCount, uiTmpCount,m_uiQuoPktGBytes*1024+m_uiQuoPktBytes/MBYTES,dlQuoPerPkt,uiBytesPerPkt,m_uiLastBandWidth,m_uiMaxBandWidth,m_uiMinBandWidth,m_uiAvgBandWidth);
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


//发布历史、资讯、实时数据
int CDeliverMgr::HandleHistoryData(CSamplerPacket& pkt)
{
    //CRLog(E_APPINFO, "forward info data.");
    
    m_deqCondMutex.Lock();

	// 按NodeID发送报文
	unsigned int uiTmpCount = 0;

	map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();

	if (!mapSubscri.empty())
	{
			for(map<unsigned int,SUB_CONTEXT>::iterator it = mapSubscri.begin();
				it != mapSubscri.end(); ++it)
			{
				CSamplerPacket &oPkt = dynamic_cast<CSamplerPacket &>(pkt);

				CMessageImpl &msg = dynamic_cast<CMessageImpl &>(oPkt.GetMsg());	

				msg.SetField(MSG_NODE_ID, it->second.nodeId);

				m_pQuoSvrCpMgr->Forward(pkt, m_ulKey);

				uiTmpCount++;
			}

	}
    else
    {
        //CRLog(E_APPINFO, "mapSubscri is empty.");
    }

	//
	m_deqCondMutex.Unlock();

	return 0;
}


//发送首次订阅完整行情
int CDeliverMgr::HandleFirstQuotation(string sKey,unsigned int uiNodeID)
{
	try
	{
		string sQuotationVal;
		QUOTATION stQuotation = {0};
		CMemData::Instance()->GetQuotationTbl().GetQuotation(sKey, stQuotation);

		unsigned int uiMarket = stQuotation.m_CodeInfo.m_usMarketType;
		uiMarket = htonl(uiMarket);

		if (uiMarket == 0)
		{
			CRLog(E_ERROR,"type err,id:%s", stQuotation.m_CodeInfo.m_acCode);
			return -1;
		}

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

		if (0 != m_pQuoSvrCpMgr)
		{
			m_pQuoSvrCpMgr->Forward(oPkt,m_ulKey);
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

		if ((m_uiFwdCount - 1) % 2000 == 0)
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
				int nInterval = m_oRecTime.IntervalToNow();
				if (nInterval >= 30)
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

void CDeliverMgr::HandleHistoryData(CPacket &pkt, unsigned int uiNodeId)
{
    //
    m_deqCondMutex.Lock();
    
    // 按NodeID发送报文
    unsigned int uiTmpCount = 0;
    
    map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();
    
    if (!mapSubscri.empty())
    {
        for(map<unsigned int,SUB_CONTEXT>::iterator it = mapSubscri.begin();
            it != mapSubscri.end(); ++it)
        {
            if (uiNodeId == it->second.nodeId)
            {
                m_pQuoSvrCpMgr->Forward(pkt, m_ulKey);

                uiTmpCount++;
            }
        }
    }

    //
    m_deqCondMutex.Unlock();
}

void CDeliverMgr::HandleInfoData(CPacket &pkt, unsigned int uiNodeId)
{
    //
    m_deqCondMutex.Lock();

    // 按NodeID发送报文
    unsigned int uiTmpCount = 0;

    map<unsigned int,SUB_CONTEXT> mapSubscri = CMemData::Instance()->GetSubscriberTbl().GetSubscriberMap();

    if (!mapSubscri.empty())
    {
        if (MAX_UINT == uiNodeId)
        {
            for(map<unsigned int,SUB_CONTEXT>::iterator it = mapSubscri.begin();
                it != mapSubscri.end(); ++it)
            {
                CSamplerPacket &oPkt = dynamic_cast<CSamplerPacket &>(pkt);

                CMessageImpl &msg = dynamic_cast<CMessageImpl &>(oPkt.GetMsg());	

                msg.SetField(MSG_NODE_ID, it->second.nodeId);

                m_pQuoSvrCpMgr->Forward(pkt, m_ulKey);

                uiTmpCount++;
            }
        }
        else
        {
            for(map<unsigned int,SUB_CONTEXT>::iterator it = mapSubscri.begin();
                it != mapSubscri.end(); ++it)
            {
                if (uiNodeId == it->second.nodeId)
                {
                    m_pQuoSvrCpMgr->Forward(pkt, m_ulKey);

                    uiTmpCount++;
                }
            }
        }
    }

    //
    m_deqCondMutex.Unlock();
}

int CDeliverMgr::Query(CNMO& oNmo)
{
	if (oNmo.m_sOid == gc_sFwdCount)
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
	return 0;
}