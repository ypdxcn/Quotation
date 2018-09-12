#include "Translator.h"
#include "SamplerCpMgr.h"
#include "SamplerPacket.h"
#include "Encode.h"


CTranslator::CTranslator(void)
:m_pSamplerCpMgr(0)
,m_pCfg(0)
,m_uiFwdCount(0)
,m_uiInCount(0)
{
}

CTranslator::~CTranslator(void)
{
}

//初始化配置
int CTranslator::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	CConfig* pConfig = m_pCfg->GetCfgGlobal();
	//pConfig->GetProperty();	
	string sUdp_ip_port;

	pConfig->GetProperty("udp_ip_port",sUdp_ip_port);
	vector<string> vIpPort = strutils::explode(",",sUdp_ip_port);
	
	vector<string>::const_iterator it;
	for (it = vIpPort.begin(); it != vIpPort.end(); ++it)
	{
		vector<string> vPair = strutils::explode(":",(*it));
		if (vPair.size() == 2)
		{
			string sIp = trim(vPair[0]);
			unsigned short usPort = strutils::FromString<unsigned short>(vPair[1]);
			if (sIp != "" && usPort != 0)
			{
				UDP_INFO stInfo;
				stInfo.sIp = sIp;
				stInfo.usPort = usPort;
				stInfo.sSocketSendUdp = socket(AF_INET, SOCK_DGRAM, 0);

				memset(&stInfo.stAddr, 0x00,sizeof(stInfo.stAddr));
				stInfo.stAddr.sin_family = AF_INET;
				stInfo.stAddr.sin_port = htons(usPort);
				stInfo.stAddr.sin_addr.s_addr = inet_addr(sIp.c_str());
				m_vUdpInfo.push_back(stInfo);
			}
		}
	}

	return 0;
}

//启动线程
int CTranslator::Start()
{
	//启动调度线程
	BeginThread();
	return 0;
}

//停止线程
void CTranslator::Stop()
{
	//停止调度线程
	CRLog(E_APPINFO,"%s","Stop Translator Thread");
	EndThread();
}

//清理资源
void CTranslator::Finish()
{
	m_deqQuotation.clear();

	for (map<string, QUOTATION*>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
	{
		delete (*it).second;
	}
	m_mapQuotation.clear();

	for (vector<UDP_INFO>::const_iterator it = m_vUdpInfo.begin(); it != m_vUdpInfo.end(); ++it)
	{
		closesocket((*it).sSocketSendUdp);
	}
	m_vUdpInfo.clear();
}

void CTranslator::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pSamplerCpMgr = dynamic_cast<CSamplerCpMgr*>(pCpMgr);
}


int CTranslator::SendPacket(CPacket &pkt)
{
	try
	{
		CSamplerPacket & pktQuotation = dynamic_cast<CSamplerPacket&>(pkt);
				
		m_deqCondMutex.Lock();
		m_deqQuotation.push_back(pktQuotation);
		m_deqCondMutex.Signal();
		m_deqCondMutex.Unlock();
		m_uiInCount++;
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

int CTranslator::ThreadEntry()
{
	try
	{
		while(!m_bEndThread)
		{
			m_deqCondMutex.Lock();
			while(m_deqQuotation.empty() && !m_bEndThread)
				m_deqCondMutex.Wait();

			if (m_bEndThread)
			{
				m_deqCondMutex.Unlock();
				break;
			}

			CSamplerPacket& pktSrc = m_deqQuotation.front();
			m_deqCondMutex.Unlock();

			try
			{
				Translate(pktSrc);
			}
			catch(...)
			{
				CRLog(E_CRITICAL,"%s","Unknown exception");
			}

			m_deqCondMutex.Lock();
			m_deqQuotation.pop_front();
			m_deqCondMutex.Unlock();
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

int CTranslator::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}

int CTranslator::Translate(CSamplerPacket& oPktSrc)
{
	string sCmdID = oPktSrc.GetCmdID();
	unsigned int ulCmdID = strutils::FromString<unsigned int>(sCmdID);

    // added by Jerry Lee, 2010-12-24, 处理历史数据报文
    if (YL_HISTORYDATA == ulCmdID)
    {
        return m_pSamplerCpMgr->ToHisDataQueue(oPktSrc);
    }

    // added by Jerry Lee, 2011-1-19, 处理tick数据报文
    if (YL_TICKDATA == ulCmdID)
    {
        return m_pSamplerCpMgr->ToTickDataQueue(oPktSrc);
    }

    // added by Jerry Lee, 2011-2-24, 处理资讯数据报文
    if (YL_INFODATA == ulCmdID)
    {
        return m_pSamplerCpMgr->ToInfoDataQueue(oPktSrc);
    }


	if (YL_QUOTATION != ulCmdID)
	{
		CRLog(E_APPINFO,"非行情包:%u",ulCmdID);
		return -1;
	}
	
	CMessageImpl& msg = dynamic_cast<CMessageImpl&>(oPktSrc.GetMsg());
	string sQuotationData;

	if (0 != msg.GetBinaryField(MSG_QUOTATION_RECS,sQuotationData))
	{
		CRLog(E_APPINFO,"读取数据错误!");
		return -1;
	}

	const unsigned char* pData = (const unsigned char*)sQuotationData.data();
	unsigned int uiMarketType = *(unsigned int*)(pData);
	uiMarketType = ntohl(uiMarketType); 
	pData += sizeof(uiMarketType);

	unsigned char cPkts = *pData;
	pData++;

	int nPriceUnit = 10 ;
	
	if (MakeMarket(uiMarketType) == FOREIGN_MARKET)
		nPriceUnit = 1;

	for (unsigned char i = 0; i < cPkts; i++)
	{
		char szCode[CODE_LEN] = {0};
		char szName[NAME_LEN] = {0};
		unsigned char ucOffset = 0;		

		unsigned char ucPktSize = *pData;
		pData++;
		//if (ucPktSize < sizeof(szCode) + 2)
		//{
		//	//CRLog(
		//	pData += ucPktSize;
		//	continue;
		//}
		
		//合约名称标志
		unsigned char ucFlag = *pData;
		pData++;
		ucOffset++;

		//合约ID
		unsigned char ucInstIDLen = *pData;
		pData++;
		ucOffset++;
		memcpy(szCode,pData,min(CODE_LEN,ucInstIDLen));
		
		pData += ucInstIDLen;
		ucOffset += ucInstIDLen;

		//合约名称
		unsigned char ucInstNameLen = NAME_LEN;
		if (1 == ucFlag)
		{
			ucInstNameLen = *pData;
			pData++;
			ucOffset++;

			memcpy(szName,pData,min(NAME_LEN,ucInstNameLen));				
			pData += ucInstNameLen;
			ucOffset += ucInstNameLen;
		}

		QUOTATION* pstQuotation = 0;
		string sKey;
		sKey.assign((const char*)(&uiMarketType), sizeof(uiMarketType));
		sKey.append(szCode, sizeof(szCode));

		map<std::string, QUOTATION*>::iterator it = m_mapQuotation.find(sKey);
		bool blFind = false;
		if(it != m_mapQuotation.end())
		{//找到，只做更新
			pstQuotation = it->second;
			blFind = true;
		}
		else
		{//没有，则加入	
			pstQuotation = new QUOTATION;
			memset(pstQuotation, 0x00, sizeof(QUOTATION));
			memcpy(pstQuotation->m_CodeInfo.m_acCode, szCode, min(sizeof(pstQuotation->m_CodeInfo.m_acCode),ucInstIDLen));
			memcpy(pstQuotation->m_CodeInfo.m_acName, szName, min(sizeof(pstQuotation->m_CodeInfo.m_acName),ucInstNameLen));
			pstQuotation->m_CodeInfo.m_usMarketType = uiMarketType;
		}
		
		unsigned int uiValue = 0;
		while (ucOffset < ucPktSize)
		{
			unsigned char ucByte = *pData;
			unsigned char ucField = (ucByte & 0xFC) >> 2;
			unsigned char ucFldLen = (ucByte & 0x03) + 1;
			if(ucOffset + 1 + ucFldLen > ucPktSize)
			{
				CRLog(E_ERROR,"行情报文长度异常!");
				return -1;
			}
			pData++;
			ucOffset++;

			uiValue = 0;
			switch (ucFldLen)
			{
			case 1:
				memcpy(((char*)&uiValue)+3, pData, 1);
				break;
			case 2:
				memcpy(((char*)&uiValue)+2, pData, 2);
				break;
			case 3:
				memcpy(((char*)&uiValue)+1, pData, 3);
				break;
			case 4:
				memcpy((char*)&uiValue, pData, 4);
				break;
			default:
				break;
			}
			uiValue = ntohl(uiValue);
			pData += ucFldLen;
			ucOffset += ucFldLen;

			switch (ucField)
			{
			case FIELDKEY_LASTSETTLE:
				pstQuotation->m_uiLastSettle = uiValue*nPriceUnit;
				break;
			case FIELDKEY_LASTCLOSE:
				pstQuotation->m_uilastClose = uiValue*nPriceUnit;
				break;
			case FIELDKEY_OPEN:
				pstQuotation->m_uiOpenPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_HIGH:
				pstQuotation->m_uiHigh = uiValue*nPriceUnit;
				break;   
			case FIELDKEY_LOW:
				pstQuotation->m_uiLow = uiValue*nPriceUnit;
				break;  
			case FIELDKEY_LAST:
				pstQuotation->m_uiLast = uiValue*nPriceUnit ;
				break;
			case FIELDKEY_CLOSE:
				pstQuotation->m_uiClose = uiValue*nPriceUnit;
				break;			
			case FIELDKEY_SETTLE:
				pstQuotation->m_uiSettle = uiValue*nPriceUnit;
				break;
			case FIELDKEY_BID1:
				pstQuotation->m_Bid[0].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_BIDLOT1:
				pstQuotation->m_Bid[0].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_BID2:
				pstQuotation->m_Bid[1].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_BIDLOT2:
				pstQuotation->m_Bid[1].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_BID3:
				pstQuotation->m_Bid[2].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_BIDLOT3:
				pstQuotation->m_Bid[2].m_uiVol = uiValue*1000;
				break;		
			case FIELDKEY_BID4:
				pstQuotation->m_Bid[3].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_BIDLOT4:
				pstQuotation->m_Bid[3].m_uiVol = uiValue*1000;
				break;		
			case FIELDKEY_BID5:
				pstQuotation->m_Bid[4].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_BIDLOT5:
				pstQuotation->m_Bid[4].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_ASK1:
				pstQuotation->m_Ask[0].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_ASKLOT1:
				pstQuotation->m_Ask[0].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_ASK2:
				pstQuotation->m_Ask[1].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_ASKLOT2:
				pstQuotation->m_Ask[1].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_ASK3:
				pstQuotation->m_Ask[2].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_ASKLOT3:
				pstQuotation->m_Ask[2].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_ASK4:
				pstQuotation->m_Ask[3].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_ASKLOT4:
				pstQuotation->m_Ask[3].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_ASK5:
				pstQuotation->m_Ask[4].m_uiPrice = uiValue*nPriceUnit;
				break;
			case FIELDKEY_ASKLOT5:
				pstQuotation->m_Ask[4].m_uiVol = uiValue*1000;
				break;
			case FIELDKEY_VOLUME:
				pstQuotation->m_uiVolume = uiValue;
				break;
			case FIELDKEY_WEIGHT:
				pstQuotation->m_uiWeight = uiValue;
				break;
			case FIELDKEY_HIGHLIMIT:
				pstQuotation->m_uiHighLimit = uiValue*nPriceUnit;
				break;
			case FIELDKEY_LOWLIMIT:
				pstQuotation->m_uiLowLimit = uiValue*nPriceUnit;
				break;
			case FIELDKEY_POSI:
				pstQuotation->m_uiChiCangLiang = uiValue;
				break;
			case FIELDKEY_UPDOWN: // 不需要处理
				break;
			case FIELDKEY_TURNOVER:			
				//以W为单位
				//pstQuotation->m_dbTurnOver = dbValue / 1000 / 10000;
				pstQuotation->m_uiTurnOver = uiValue / 100;
				break;
			case FIELDKEY_AVERAGE:
				pstQuotation->m_uiAverage = uiValue*nPriceUnit;
				break;
			case FIELDKEY_SEQUENCENO:
				pstQuotation->m_uiSeqNo = uiValue*1000;
				break;
			case FIELDKEY_QUOTETIME: //次字段处理需要再确认
				pstQuotation->m_uiTime = uiValue;
				break;
			case FIELDKEY_UPDOWNRATE: // 不需要处理
				break;
			case FIELDKEY_QUOTEDATE:
				pstQuotation->m_uiDate = uiValue;
				break;
			case FIELDKEY_UNIT:
				pstQuotation->m_CodeInfo.m_uiUnit = uiValue;
				break;
			default:
				CRLog(E_ERROR,"行情报文字段序号[%d]不存在!",ucField);
				break;
			}
		}

		//SendUdpPkt(*pstQuotation);		
		if (0 != m_pSamplerCpMgr)
			m_pSamplerCpMgr->ToXQueue(*pstQuotation);
		
		if (!blFind)
			m_mapQuotation.insert(pair<string, QUOTATION*>(sKey, pstQuotation));
		//else
		//	m_mapQuotation[sKey] = pstQuotation;

		m_uiFwdCount++;
		if ((m_uiFwdCount - 1) % 2000 == 0)
		{
			CRLog(E_STATICS,"FWD(%u): InstID(%s),Last:%u",m_uiFwdCount, pstQuotation->m_CodeInfo.m_acCode,pstQuotation->m_uiLast);
		}
	}

	return 0;
}

int CTranslator::SendUdpPkt(QUOTATION& stQuotation)
{
	SendUDPDataItem stUdpPack;
	memset(&stUdpPack, 0, sizeof(SendUDPDataItem));
	TranslateCode(stUdpPack.m_ciStockCode.m_cCode, stQuotation.m_CodeInfo.m_acCode);
	strncpy(stUdpPack.m_cStockName, stQuotation.m_CodeInfo.m_acName,STOCK_NAME_SIZE);


	stUdpPack.m_lDataLen = sizeof(SendUDPDataItem);
	stUdpPack.m_szFlag[0] = 'H';
	stUdpPack.m_szFlag[1] = 'S';
	stUdpPack.m_szFlag[2] = 'H';
	stUdpPack.m_szFlag[3] = 'S';
	stUdpPack.m_ciStockCode.m_cCodeType = (HSMarketDataType) stQuotation.m_CodeInfo.m_usMarketType;
	HSMarketDataType hsMarketType = (HSMarketDataType) stQuotation.m_CodeInfo.m_usMarketType;

	//现货
	if (IS_CURR_GOLD(stUdpPack.m_ciStockCode.m_cCode, hsMarketType)||
		MakeMarket(hsMarketType) == MakeMarket(STOCK_MARKET))
	{
		stUdpPack.m_sData.m_nowData.m_lOpen = (long) (stQuotation.m_uiOpenPrice);
		stUdpPack.m_sData.m_nowData.m_lMaxPrice = (long) (stQuotation.m_uiHigh);
		stUdpPack.m_sData.m_nowData.m_lMinPrice = (long) (stQuotation.m_uiLow);
		stUdpPack.m_sData.m_nowData.m_lNewPrice = (long) (stQuotation.m_uiLast);
        stUdpPack.m_sData.m_nowData.m_lTotal    = (long) (stQuotation.m_uiVolume);	
		
		//stUdpPack.m_sData.m_nowData.m_fAvgPrice = (float)(stQuotation.m_uiTurnOver/100.00);
		stUdpPack.m_sData.m_nowData.m_fAvgPrice = (float)(stQuotation.m_uiTurnOver);




		stUdpPack.m_sData.m_nowData.m_lBuyCount1 =(long) (stQuotation.m_Bid[0].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lBuyCount2 =(long) (stQuotation.m_Bid[1].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lBuyCount3 =(long) (stQuotation.m_Bid[2].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lBuyCount4 =(long) (stQuotation.m_Bid[3].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lBuyCount5 =(long) (stQuotation.m_Bid[4].m_uiVol);

		stUdpPack.m_sData.m_nowData.m_lSellCount1 = (long) (stQuotation.m_Ask[0].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lSellCount2 = (long) (stQuotation.m_Ask[1].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lSellCount3 = (long) (stQuotation.m_Ask[2].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lSellCount4 = (long) (stQuotation.m_Ask[3].m_uiVol);
		stUdpPack.m_sData.m_nowData.m_lSellCount5 = (long) (stQuotation.m_Ask[4].m_uiVol);

		stUdpPack.m_sData.m_nowData.m_lBuyPrice1 =(long) (stQuotation.m_Bid[0].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lBuyPrice2 =(long) (stQuotation.m_Bid[1].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lBuyPrice3 =(long) (stQuotation.m_Bid[2].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lBuyPrice4 =(long) (stQuotation.m_Bid[3].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lBuyPrice5 =(long) (stQuotation.m_Bid[4].m_uiPrice);

		stUdpPack.m_sData.m_nowData.m_lSellPrice1 = (long) (stQuotation.m_Ask[0].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lSellPrice2 = (long) (stQuotation.m_Ask[1].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lSellPrice3 = (long) (stQuotation.m_Ask[2].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lSellPrice4 = (long) (stQuotation.m_Ask[3].m_uiPrice);
		stUdpPack.m_sData.m_nowData.m_lSellPrice5 = (long) (stQuotation.m_Ask[4].m_uiPrice);

		stUdpPack.m_lPreClose = (long) (stQuotation.m_uilastClose);

		//stUdpPack.m_sData.m_nowData.m_nHand = (long) (pReport->m_fTickAll * nPriceUnit);
	}
	else if(MakeMarket(hsMarketType) == MakeMarket(FOREIGN_MARKET))
	{

		stUdpPack.m_lPreClose = (long) (stQuotation.m_uilastClose);

		stUdpPack.m_sData.m_whData.m_lOpen = (long) (stQuotation.m_uiOpenPrice);
		stUdpPack.m_sData.m_whData.m_lMaxPrice = (long) (stQuotation.m_uiHigh);
		stUdpPack.m_sData.m_whData.m_lMinPrice = (long) (stQuotation.m_uiLow);
		stUdpPack.m_sData.m_whData.m_lNewPrice = (long) (stQuotation.m_uiLast);

		stUdpPack.m_sData.m_whData.m_lBuyPrice = (long) (stQuotation.m_Bid[0].m_uiPrice);
		stUdpPack.m_sData.m_whData.m_lSellPrice = (long) (stQuotation.m_Ask[0].m_uiPrice);


	}
	else
	{
		stUdpPack.m_sData.m_qhData.m_lOpen = (long) (stQuotation.m_uiOpenPrice);
		stUdpPack.m_sData.m_qhData.m_lMaxPrice = (long) (stQuotation.m_uiHigh);
		stUdpPack.m_sData.m_qhData.m_lMinPrice = (long) (stQuotation.m_uiLow);
		stUdpPack.m_sData.m_qhData.m_lNewPrice = (long) (stQuotation.m_uiLast);

		stUdpPack.m_sData.m_qhData.m_lTotal = (long) (stQuotation.m_uiVolume);
		stUdpPack.m_sData.m_qhData.m_lChiCangLiang = (long) (stQuotation.m_uiChiCangLiang);

		stUdpPack.m_sData.m_qhData.m_lBuyCount1 =(long) (stQuotation.m_Bid[0].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lBuyCount2 =(long) (stQuotation.m_Bid[1].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lBuyCount3 =(long) (stQuotation.m_Bid[2].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lBuyCount4 =(long) (stQuotation.m_Bid[3].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lBuyCount5 =(long) (stQuotation.m_Bid[4].m_uiVol);

		stUdpPack.m_sData.m_qhData.m_lSellCount1 = (long) (stQuotation.m_Ask[0].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lSellCount2 = (long) (stQuotation.m_Ask[1].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lSellCount3 = (long) (stQuotation.m_Ask[2].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lSellCount4 = (long) (stQuotation.m_Ask[3].m_uiVol);
		stUdpPack.m_sData.m_qhData.m_lSellCount5 = (long) (stQuotation.m_Ask[4].m_uiVol);

		stUdpPack.m_sData.m_qhData.m_lBuyPrice1 =(long) (stQuotation.m_Bid[0].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lBuyPrice2 =(long) (stQuotation.m_Bid[1].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lBuyPrice3 =(long) (stQuotation.m_Bid[2].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lBuyPrice4 =(long) (stQuotation.m_Bid[3].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lBuyPrice5 =(long) (stQuotation.m_Bid[4].m_uiPrice);

		stUdpPack.m_sData.m_qhData.m_lSellPrice1 = (long) (stQuotation.m_Ask[0].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lSellPrice2 = (long) (stQuotation.m_Ask[1].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lSellPrice3 = (long) (stQuotation.m_Ask[2].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lSellPrice4 = (long) (stQuotation.m_Ask[3].m_uiPrice);
		stUdpPack.m_sData.m_qhData.m_lSellPrice5 = (long) (stQuotation.m_Ask[4].m_uiPrice);

		stUdpPack.m_sData.m_qhData.m_lJieSuanPrice =  (long) (stQuotation.m_uiSettle);
		stUdpPack.m_sData.m_qhData.m_lPreJieSuanPrice = (long) (stQuotation.m_uiLastSettle);

		stUdpPack.m_sData.m_qhData.m_lPreClose = (long) (stQuotation.m_uilastClose);

		stUdpPack.m_sData.m_qhData.m_lCurrentCLOSE = (long) (stQuotation.m_uiClose);	// 今收盘
		stUdpPack.m_sData.m_qhData.m_lUPPER_LIM = (long) (stQuotation.m_uiHighLimit);		// 涨停板
		stUdpPack.m_sData.m_qhData.m_lLOWER_LIM = (long) (stQuotation.m_uiLowLimit);		// 跌停板

		stUdpPack.m_sData.m_qhData.m_lAvgPrice = (long)stQuotation.m_uiTurnOver;

		stUdpPack.m_lPreClose = (long) (stQuotation.m_uilastClose);
		//	stUdpPack.m_sData.m_qhData.m_nHand = (long) (pReport->m_fTickAll * nPriceUnit);
	}

	//直接发出去
	m_uiFwdCount++;
	unsigned int uiTmp = m_uiFwdCount;
	for (vector<UDP_INFO>::const_iterator it = m_vUdpInfo.begin(); it != m_vUdpInfo.end(); ++it)
	{
		uiTmp++;
		if (SOCKET_ERROR == sendto((*it).sSocketSendUdp, (const char*)&stUdpPack, stUdpPack.m_lDataLen, 0,(struct sockaddr *)&((*it).stAddr), sizeof((*it).stAddr)))
		{
			CRLog(E_ERROR,"%s","Send Err sock");
		}
		else
		{
			
		}
	}

	if ((m_uiFwdCount - 1) % 10000 == 0)
	{
		CRLog(E_DEBUG,"Quotation: InstID(%s),Last:%u, %u-%u",stQuotation.m_CodeInfo.m_acCode,stQuotation.m_uiLast, m_uiFwdCount, uiTmp);
	}

	return 0;
}

void CTranslator::TranslateCode(char* pcDestCode, const char* pccOriCode)
{

	if (!strncmp(pccOriCode, "Au(T+D)", 7))
	{
		memcpy(pcDestCode, "AUTD", 4);		
	}
	else if (!strncmp(pccOriCode, "Au(T+5)", 7))
	{
		memcpy(pcDestCode, "AUT5", 4);
	}
	else if (!strncmp(pccOriCode, "Au(T+N1)", 8))
	{
		memcpy(pcDestCode, "AUTN1", 5);
	}
	//Mod wjj 2010-6-24 AUTN5 修改为　AUTN2
	else if (!strncmp(pccOriCode, "Au(T+N2)", 8))
	{
		memcpy(pcDestCode, "AUTN2", 5);
	}
	else if (!strncmp(pccOriCode, "Ag(T+D)", 7))
	{
		memcpy(pcDestCode, "AGTD", 4);
	}

	else if (!strcmp(pccOriCode, "Au99.95"))
	{
		memcpy(pcDestCode, "AU9995", 6);
	}
	else if (!strcmp(pccOriCode, "Au99.99"))
	{
		memcpy(pcDestCode, "AU99999", 6);
	}
	else if (!strcmp(pccOriCode, "Pt99.95"))
	{
        memcpy(pcDestCode, "PT9995", 6);
	}
	else if (!strcmp(pccOriCode, "Au100g"))
	{
		memcpy(pcDestCode, "AU100", 5);
	}
	else if (!strcmp(pccOriCode, "Au50g"))
	{
        memcpy(pcDestCode, "AU50", 4);
	}
	else if (!strcmp(pccOriCode, "Ag99.9"))
	{
		memcpy(pcDestCode, "Ag999", 5);
	}
	else if (!strcmp(pccOriCode, "Ag99.99"))
	{
		memcpy(pcDestCode, "AG9999", 6);
	}
	else
		strcpy(pcDestCode,pccOriCode);
}