#include "HisDataHandler.h"
#include "QuoSvrCpMgr.h"

CHisDataHandler::CHisDataHandler(void)
:m_pCpMgr(0)
,m_pCfg(0)
{
}

CHisDataHandler::~CHisDataHandler(void)
{
}

//初始化配置，创建线程池
int CHisDataHandler::Init(CConfig* pCfg)
{
	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	//CConfig* pConfig = m_pCfg->GetCfgGlobal();

	return 0;
}

//启动调度线程及工作线程池
int CHisDataHandler::Start()
{
	//启动调度线程
	BeginThread();
	return 0;
}

//停止工作线程池及调度线程
void CHisDataHandler::Stop()
{
	//停止调度线程
	CRLog(E_APPINFO,"%s","Stop RiskHandler Thread");
	EndThread();
}

//清理资源
void CHisDataHandler::Finish()
{
	m_deqQuotation.clear();
}

void CHisDataHandler::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pCpMgr = dynamic_cast<CQuoSvrCpMgr*>(pCpMgr);
}


int CHisDataHandler::SendPacket(CPacket &pkt)
{
	try
	{
		CSamplerPacket & pktQuotation = dynamic_cast<CSamplerPacket&>(pkt);
				
		m_deqCondMutex.Lock();
		m_deqQuotation.push_back(pktQuotation);
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

int CHisDataHandler::ThreadEntry()
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

			CSamplerPacket& pkt = m_deqQuotation.front();
			m_deqCondMutex.Unlock();

			SaveQuotation(pkt);

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

//保存
int CHisDataHandler::SaveQuotation(CSamplerPacket& oPktSrc)
{
	try
	{
		string sCmdID = oPktSrc.GetCmdID();
		unsigned int ulCmdID = strutils::FromString<unsigned int>(sCmdID);
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

		if (uiMarketType == 0)
		{
			CRLog(E_ERROR, "type 0 err");
			return -1;
		}		

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

			QUOTATION stQuotation = {0};
			string sKey;
			sKey.assign((const char*)(&uiMarketType), sizeof(uiMarketType));
			sKey.append(szCode, sizeof(szCode));

			if (0 != CMemData::Instance()->GetQuotationTbl().GetQuotation(sKey, stQuotation))
			{
				stQuotation.m_CodeInfo.m_usMarketType = uiMarketType;
				memcpy(stQuotation.m_CodeInfo.m_acCode, szCode, min(sizeof(stQuotation.m_CodeInfo.m_acCode),ucInstIDLen));
				memcpy(stQuotation.m_CodeInfo.m_acName, szName, min(sizeof(stQuotation.m_CodeInfo.m_acName),ucInstNameLen));
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
					stQuotation.m_uiLastSettle = uiValue;
					break;
				case FIELDKEY_LASTCLOSE:
					stQuotation.m_uilastClose = uiValue;
					break;
				case FIELDKEY_OPEN:
					stQuotation.m_uiOpenPrice = uiValue;
					break;
				case FIELDKEY_HIGH:
					stQuotation.m_uiHigh = uiValue;
					break;   
				case FIELDKEY_LOW:
					stQuotation.m_uiLow = uiValue;
					break;  
				case FIELDKEY_LAST:
					stQuotation.m_uiLast = uiValue;
					break;
				case FIELDKEY_CLOSE:
					stQuotation.m_uiClose = uiValue;
					break;			
				case FIELDKEY_SETTLE:
					stQuotation.m_uiSettle = uiValue;
					break;
				case FIELDKEY_BID1:
					stQuotation.m_Bid[0].m_uiPrice = uiValue;
					break;
				case FIELDKEY_BIDLOT1:
					stQuotation.m_Bid[0].m_uiVol = uiValue;
					break;
				case FIELDKEY_BID2:
					stQuotation.m_Bid[1].m_uiPrice = uiValue;
					break;
				case FIELDKEY_BIDLOT2:
					stQuotation.m_Bid[1].m_uiVol = uiValue;
					break;
				case FIELDKEY_BID3:
					stQuotation.m_Bid[2].m_uiPrice = uiValue;
					break;
				case FIELDKEY_BIDLOT3:
					stQuotation.m_Bid[2].m_uiVol = uiValue;
					break;		
				case FIELDKEY_BID4:
					stQuotation.m_Bid[3].m_uiPrice = uiValue;
					break;
				case FIELDKEY_BIDLOT4:
					stQuotation.m_Bid[3].m_uiVol = uiValue;
					break;		
				case FIELDKEY_BID5:
					stQuotation.m_Bid[4].m_uiPrice = uiValue;
					break;
				case FIELDKEY_BIDLOT5:
					stQuotation.m_Bid[4].m_uiVol = uiValue;
					break;
				case FIELDKEY_ASK1:
					stQuotation.m_Ask[0].m_uiPrice = uiValue;
					break;
				case FIELDKEY_ASKLOT1:
					stQuotation.m_Ask[0].m_uiVol = uiValue;
					break;
				case FIELDKEY_ASK2:
					stQuotation.m_Ask[1].m_uiPrice = uiValue;
					break;
				case FIELDKEY_ASKLOT2:
					stQuotation.m_Ask[1].m_uiVol = uiValue;
					break;
				case FIELDKEY_ASK3:
					stQuotation.m_Ask[2].m_uiPrice = uiValue;
					break;
				case FIELDKEY_ASKLOT3:
					stQuotation.m_Ask[2].m_uiVol = uiValue;
					break;
				case FIELDKEY_ASK4:
					stQuotation.m_Ask[3].m_uiPrice = uiValue;
					break;
				case FIELDKEY_ASKLOT4:
					stQuotation.m_Ask[3].m_uiVol = uiValue;
					break;
				case FIELDKEY_ASK5:
					stQuotation.m_Ask[4].m_uiPrice = uiValue;
					break;
				case FIELDKEY_ASKLOT5:
					stQuotation.m_Ask[4].m_uiVol = uiValue;
					break;
				case FIELDKEY_VOLUME:
					stQuotation.m_uiVolume = uiValue;
					break;
				case FIELDKEY_WEIGHT:
					stQuotation.m_uiWeight = uiValue;
					break;
				case FIELDKEY_HIGHLIMIT:
					stQuotation.m_uiHighLimit = uiValue;
					break;
				case FIELDKEY_LOWLIMIT:
					stQuotation.m_uiLowLimit = uiValue;
					break;
				case FIELDKEY_POSI:
					stQuotation.m_uiChiCangLiang = uiValue;
					break;
				case FIELDKEY_UPDOWN: // 不需要处理
					break;
				case FIELDKEY_TURNOVER:			
					//以W为单位
					stQuotation.m_uiTurnOver = uiValue;
					break;
				case FIELDKEY_AVERAGE:
					stQuotation.m_uiAverage = uiValue;
					break;
				case FIELDKEY_SEQUENCENO:
					stQuotation.m_uiSeqNo = uiValue;
					break;
				case FIELDKEY_QUOTETIME: //次字段处理需要再确认
					stQuotation.m_uiTime = uiValue;
					break;
				case FIELDKEY_UPDOWNRATE: // 不需要处理
					break;
				case FIELDKEY_QUOTEDATE:
					stQuotation.m_uiDate = uiValue;
					break;
				case FIELDKEY_UNIT:
					stQuotation.m_CodeInfo.m_uiUnit = uiValue;
					break;
				default:
					CRLog(E_ERROR,"行情报文字段序号[%d]不存在!",ucField);
					break;
				}
			}

			CMemData::Instance()->GetQuotationTbl().SetQuotation(sKey, stQuotation);
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

int CHisDataHandler::End()
{
	m_deqCondMutex.Lock();
	m_deqCondMutex.Signal();
	m_deqCondMutex.Unlock();

	CRLog(E_APPINFO,"%s","ServiceHanlder thread wait end");
	Wait();
	return 0;
}