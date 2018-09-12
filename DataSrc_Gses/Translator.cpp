#include <cassert>
#include "Translator.h"
#include "DataSrcCpMgr.h"

//#define SGE_MARKET	        0x7000
//7100 第一个品种
//7102 第二个品种
#define SGE_MARKET	        0x6300//放在“国际黄金”栏目里面
#define SY_BJ_CURR			0x0100


CTranslator::CTranslator(void)
:m_pDataSrcCpMgr(0)
,m_pCfg(0)
,m_ucInstState(I_INIT)
,m_uiInCount(0)
,m_nZipOut(0)
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

    // 增加读取过滤设置, Jerry Lee, 2012-3-26
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

	m_mapQuotation.clear();
}


void CTranslator::Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey)
{
	m_ulKey = ulKey; 
	m_pDataSrcCpMgr = dynamic_cast<CDataSrcCpMgr*>(pCpMgr);
}

int CTranslator::SendPacket(CPacket &pkt)
{
	try
	{
		CBroadcastPacket & pktQuotation = dynamic_cast<CBroadcastPacket&>(pkt);
		string sCmdID = pktQuotation.GetCmdID();
		if (sCmdID == "onRecvRtnDeferInstStateUpdate"
			|| sCmdID == "onRecvRtnInstStateUpdate"
			|| sCmdID == "onRecvRtnForwardInstStateUpdate"
			|| sCmdID == "onRecvRtnSpotInstStateUpdate")
		{
			HandleInstState(pktQuotation);
			return 0;
		}
		else if (sCmdID == "onSysStatChange")
		{
			HandleSysStat(pktQuotation);
			return 0;
		}
				
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

			CBroadcastPacket& pktSrc = m_deqQuotation.front();
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

int CTranslator::Translate(CBroadcastPacket& oPktSrc)
{
	try
	{
        // 临时版本，在星期天晚上九点至星期一两点半之间的数据不转发, Jerry Lee, 2012-3-2
        /*
        在配置文件中增加如下信息
        FILTER.week1=12:00,15:59;21:00,23:59;00:00,5:59
        FILTER.week2=12:00,15:59;21:00,23:59;00:00,5:59
        FILTER.week3=12:00,15:59;21:00,23:59;00:00,5:59
        FILTER.week4=21:00,23:59;00:00,5:59;12:00,15:11
        FILTER.week5=12:00,15:59;21:00,23:59;00:00,5:59
        FILTER.week6=12:00,15:59;21:00,23:59;00:00,5:59
        FILTER.week7=12:00,15:59;21:00,23:59;00:00,5:59
        */
	    SYSTEMTIME stNow;
        GetLocalTime(&stNow);
        int i = (stNow.wDayOfWeek+6)%7;

        if (m_timeFilters[i].isFilter(stNow.wHour, stNow.wMinute))
        {
            CRLog(E_APPINFO,"抛弃非交易时间段行情数据!");

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

        string sZipVal;
		string sInstID;
		oPktSrc.GetParameterVal("prod_code",sInstID);//instID

	//	CRLog(E_DEBUG,"instID = [%s],RecvMsg = [%s]",sInstID.c_str(),oPktSrc.Print().c_str());

		map<std::string, QUOTATION>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			QUOTATION& stQuotation = (*it).second;
			if (0 != oPktSrc.GetParameterVal("sZipBuff",sZipVal))
			{
				vector<string> vKey = oPktSrc.GetKeys();
				if (vKey.size() > 2)
				{
					if (0 != TranslateUnzipPacket(oPktSrc, stQuotation))
					{
						return -1;
					}
				}
				else
				{
					//CRLog
					return -1;
				}
			}
			else
			{
				if (0 != TranslateZipPacket(oPktSrc, stQuotation))
				{
					return -1;
				}
			}

			//由于每日收盘价行情很晚才会回送，而且大多是在收盘后，无论什么时段都转发
			//一级系统原因有时候行情比状态报文先收到
			if (0 != m_pDataSrcCpMgr )
			{

				CRLog(E_DEBUG,"instID = [%s]",sInstID.c_str());
				m_pDataSrcCpMgr->ToXQueue(stQuotation);
			}
		}
		else
		{
			QUOTATION stQuotation = {0};
			SYSTEMTIME st;
			::GetLocalTime(&st);
			stQuotation.m_uiDate = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
			stQuotation.m_uiTime = st.wHour * 10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;
			
			if (0 != oPktSrc.GetParameterVal("sZipBuff",sZipVal))
			{
				vector<string> vKey = oPktSrc.GetKeys();
				if (vKey.size() > 2)
				{
					if (0 != TranslateUnzipPacket(oPktSrc, stQuotation))
					{
						return -1;
					}
				}
				else
				{
					//CRLog
					return -1;
				}
			}
			else
			{
				if (0 != TranslateZipPacket(oPktSrc, stQuotation))
				{
					return -1;
				}
			}

			string sNewInstID = sInstID;
			ConvertInstID(sNewInstID);
			memcpy(stQuotation.m_CodeInfo.m_acCode, sNewInstID.c_str(), min(sizeof(stQuotation.m_CodeInfo.m_acCode), sNewInstID.length()));
			stQuotation.m_CodeInfo.m_usMarketType = SGE_MARKET;
			
			m_mapQuotation[sInstID] = stQuotation;

			//进程刚启动时从通讯接口机获取的全量行情根据交易时段决定是否转发
			/*if (I_INITING == m_ucInstState || I_INIT == m_ucInstState 
				|| I_END == m_ucInstState || I_PAUSE == m_ucInstState)
			{
				CRLog(E_DEBUG, "丢弃(TradeState=%c),[%s]%u-%u", m_ucInstState,stQuotation.m_CodeInfo.m_acCode,stQuotation.m_uiDate,stQuotation.m_uiTime);
				return 0;
			}
			else */if (0 != m_pDataSrcCpMgr)
			{
				m_pDataSrcCpMgr->ToXQueue(stQuotation);
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

int CTranslator::TranslateUnzipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation)
{
	//
	int nRtn = 0;
	string strTmp = "";
	if ( 0 == oPktSrc.GetParameterVal("name",strTmp) )
	{
		strncpy(stQuotation.m_CodeInfo.m_acName, strTmp.c_str(), min(strTmp.length(), sizeof(stQuotation.m_CodeInfo.m_acName)));
	}

	if (0 == oPktSrc.GetParameterVal("sequenceNo",strTmp))
	{
		unsigned int uiSeq = FromString<unsigned int>(strTmp);
		if (stQuotation.m_uiSeqNo < uiSeq)
		{
			stQuotation.m_uiSeqNo = uiSeq;
		}
		else
		{			
			CRLog(E_DEBUG,"旧的序列号：%u, 新序列号：%u,", stQuotation.m_uiSeqNo, uiSeq);
			stQuotation.m_uiSeqNo = uiSeq;

			//nRtn = -1;
		}
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("quoteDate",strTmp))
	{
		unsigned int uiDate = FromString<unsigned int>(strTmp);
		if (stQuotation.m_uiDate < uiDate)
		{
			stQuotation.m_uiDate = uiDate;
		}
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("quoteTime",strTmp))
	{
		vector<string> v = strutils::explode(":",strTmp);
		if (v.size() == 3)
		{
			int nHour = FromString<int>(v[0]);
			int nMin = FromString<int>(v[1]);
			int nSec = FromString<int>(v[2]);
			//stQuotation.m_uiTime = nHour * 10000 + nMin * 100 + nSec;
			stQuotation.m_uiTime = nHour * 10000000 + nMin * 100000 + nSec*1000 + 0;
		}
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("open",strTmp))
	{
		stQuotation.m_uiOpenPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("high",strTmp))	
	{
		stQuotation.m_uiHigh = RoundToInt<double>(FromString<double>(strTmp),4);
	}


	//kenny  2013-11-25  打开
	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("close",strTmp))
	{// 今收盘
		stQuotation.m_uiClose = RoundToInt<double>(FromString<double>(strTmp),4);
	}
	


	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("low",strTmp))	
	{
		stQuotation.m_uiLow = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	//added by kenny
	

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("lastSettle",strTmp))
	{
		stQuotation.m_uiLastSettle = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	//暂时无昨收的值
	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("close",strTmp))
	{
		stQuotation.m_uilastClose = stQuotation.m_uiLastSettle;
	}
	if (stQuotation.m_uiOpenPrice >=0 && stQuotation.m_uiOpenPrice <=0)
	{
		stQuotation.m_uiOpenPrice = stQuotation.m_uilastClose;
	}

	//kenny 2013-11-25  屏蔽
	//if (stQuotation.m_uiClose >=0 && stQuotation.m_uiClose <=0)
	//{
	//	stQuotation.m_uiClose = stQuotation.m_uilastClose;
	//}


	//string sInstID;
	//oPktSrc.GetParameterVal("prod_code",sInstID);//instID
	//if (sInstID.compare("ULP2") == 0)
	//{
	//	stQuotation.m_uilastClose = 700;
	//}
	//else
	//{
	//	stQuotation.m_uilastClose = 900000;

	//}


	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("last",strTmp))
	{		
		stQuotation.m_uiLast = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	//现价
	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("mid1",strTmp))	
	{
		stQuotation.m_uiLast = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	//end added


	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("volume",strTmp))	
	{
		stQuotation.m_uiVolume = (unsigned int)(FromString<unsigned int>(strTmp));
	}



	strTmp = "";	
	if (0 == oPktSrc.GetParameterVal("ask1",strTmp))	
	{
		stQuotation.m_Ask[0].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("ask2",strTmp))	
	{
		stQuotation.m_Ask[1].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("ask3",strTmp))	
	{
		stQuotation.m_Ask[2].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("ask4",strTmp))	
	{
		stQuotation.m_Ask[3].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("ask5",strTmp))	
	{
		stQuotation.m_Ask[4].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("askLot1",strTmp))	
	{
		stQuotation.m_Ask[0].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("askLot2",strTmp))	
	{
		stQuotation.m_Ask[1].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("askLot3",strTmp))	
	{
		stQuotation.m_Ask[2].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("askLot4",strTmp))	
	{
		stQuotation.m_Ask[3].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("askLot5",strTmp))	
	{
		stQuotation.m_Ask[4].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bid1",strTmp))	
	{
		stQuotation.m_Bid[0].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bid2",strTmp))	
	{
		stQuotation.m_Bid[1].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bid3",strTmp))	
	{
		stQuotation.m_Bid[2].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bid4",strTmp))	
	{
		stQuotation.m_Bid[3].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bid5",strTmp))	
	{
		stQuotation.m_Bid[4].m_uiPrice = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bidLot1",strTmp))	
	{
		stQuotation.m_Bid[0].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bidLot2",strTmp))	
	{
		stQuotation.m_Bid[1].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bidLot3",strTmp))	
	{
		stQuotation.m_Bid[2].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bidLot4",strTmp))	
	{
		stQuotation.m_Bid[3].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("bidLot5",strTmp))	
	{
		stQuotation.m_Bid[4].m_uiVol = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("turnOver",strTmp))
	{//再 /１００００
		//stQuotation.m_dbTurnOver = (FromString<double>(strTmp)/10000);
		stQuotation.m_uiTurnOver = RoundToInt<double>(FromString<double>(strTmp),-2);
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("highLimit",strTmp))
	{// 涨停板
		stQuotation.m_uiHighLimit = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("lowLimit",strTmp))
	{// 跌停板
		stQuotation.m_uiLowLimit = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("average",strTmp))
	{// 跌停板
		stQuotation.m_uiAverage = RoundToInt<double>(FromString<double>(strTmp),4);
	}

	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("weight",strTmp))
	{// 跌停板
		stQuotation.m_uiWeight = RoundToInt<double>(FromString<double>(strTmp),3);
	}
	
	strTmp = "";		
	if (0 == oPktSrc.GetParameterVal("Posi",strTmp))
	{
		stQuotation.m_uiChiCangLiang = (unsigned int)(FromString<unsigned int>(strTmp));
	}

	strTmp = "";
	if (0 == oPktSrc.GetParameterVal("settle",strTmp))
	{
		stQuotation.m_uiSettle = RoundToInt<double>(FromString<double>(strTmp),4);
	}


	return nRtn;
}

int CTranslator::TranslateZipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation)
{
	int nRtn = 0;
	string sZipVal;
	string sInstID;
	oPktSrc.GetParameterVal("instID",sInstID);
	oPktSrc.GetParameterVal("sZipBuff",sZipVal);

	string sTmp;
	if (0 == oPktSrc.GetParameterVal("quoteDate", sTmp))
	{
		stQuotation.m_uiDate = FromString<unsigned int>(sTmp);
	}

	//SYSTEMTIME sysTime;
	//unsigned int uiTime = 0;

	//去掉\r\n
	sZipVal = strutils::stripNewLines(sZipVal);
	//base64解码
	sZipVal = strutils::base64Decode(sZipVal);

	//int nLen = CEncode::unbase64(const_cast<char*>(sZipVal.c_str()),sZipVal.length(),aZipData);
	//aZipData 按照多个FLV串接而成 即第一字节的6bit表示FieldID,随后2bit的值+3=后续实际值占用字节数,实际值按照网络字节序

	int nIdx = 0;
	char cField;
	char cLen;
	char acValues[6];
	unsigned int uiValue;
	unsigned char* lpPointer = (unsigned char*)sZipVal.data();
	int nLen = sZipVal.length();
	char cByte;

	string sFldQuo;
	while (nIdx < nLen && nRtn != -1)
	{
		cByte = lpPointer[nIdx];
		// BIT:0 - 5
		cField = (cByte & 0xFC) >> 2 ;//11111100;
		// BIT:6 - 7   x + 3
		cLen   = (cByte & 0x03) + 3;
		if(nIdx + cLen >= nLen)
		{
			CRLog(E_ERROR,"行情报文长度异常!");
			nRtn = -1;
			return nRtn;
		}
		
		nIdx ++;
		memset(acValues, 0, 6);
		memcpy(acValues, &lpPointer[nIdx], cLen);        
		nIdx += cLen;
		double dbValue = 0.0;
		for (int i = 0; i < cLen; i++)
		{
			unsigned char chTmp = acValues[i];

			for(int j = 7; j >= 0; j --)
			{
				unsigned int nIndex = ((cLen - i - 1 ) * 8 + j);
				int nFlag = ((chTmp >> j) & 0x01);
				if(nFlag > 0)
				{
					dbValue += pow(2.0, (double)nIndex);
				}
			}
		}
		uiValue = (unsigned int)dbValue;
		switch (cField)
		{
		case FIELDKEY_LASTSETTLE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "LastSettle(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLastSettle);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLastSettle = uiValue/10;
			break;
		case FIELDKEY_LASTCLOSE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "lastClose(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uilastClose);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uilastClose = uiValue/10;
			break;
		case FIELDKEY_OPEN:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "OpenPrice(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiOpenPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiOpenPrice = uiValue/10;
			break;
		case FIELDKEY_HIGH:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "High(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiHigh);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiHigh = uiValue/10;
			break;   
		case FIELDKEY_LOW:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Low(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLow);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLow = uiValue/10;
			break;  
		case FIELDKEY_LAST:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Last(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLast);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLast = uiValue/10;
			break;
		case FIELDKEY_CLOSE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Close(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiClose);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiClose = uiValue/10;
			break;			
		case FIELDKEY_SETTLE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Settle(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiSettle);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiSettle = uiValue/10;
			break;
		case FIELDKEY_BID1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid1p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[0].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[0].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid1v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[0].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[0].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_BID2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid2p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[1].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[1].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid2v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[1].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[1].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_BID3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid3p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[2].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[2].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid3v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[2].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[2].m_uiVol = uiValue/1000;
			break;		
		case FIELDKEY_BID4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid4p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[3].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[3].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid4v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[3].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[3].m_uiVol = uiValue/1000;
			break;		
		case FIELDKEY_BID5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid5p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[4].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[4].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_BIDLOT5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Bid5v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Bid[4].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Bid[4].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask1p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[0].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[0].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT1:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask1v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[0].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[0].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask2p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[1].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[1].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT2:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask2v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[1].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[1].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask3p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[2].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[2].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT3:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask3v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[2].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[2].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask4p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[3].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[3].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT4:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask4v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[3].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[3].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_ASK5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask5p(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[4].m_uiPrice);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[4].m_uiPrice = uiValue/10;
			break;
		case FIELDKEY_ASKLOT5:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Ask5v(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_Ask[40].m_uiVol);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_Ask[4].m_uiVol = uiValue/1000;
			break;
		case FIELDKEY_VOLUME:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Volume(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiVolume);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_uiVolume = uiValue /1000;
			break;
		case FIELDKEY_WEIGHT:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Weight(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiWeight);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue);
				sFldQuo += ")|";
			}

			stQuotation.m_uiWeight = uiValue;
			break;
		case FIELDKEY_HIGHLIMIT:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "HighLimit(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiHighLimit);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiHighLimit = uiValue/10;
			break;
		case FIELDKEY_LOWLIMIT:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "LowLimit(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiLowLimit);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiLowLimit = uiValue/10;
			break;
		case FIELDKEY_POSI:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "ChiCangLiang(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiChiCangLiang);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			stQuotation.m_uiChiCangLiang = uiValue /1000;
			break;
		case FIELDKEY_UPDOWN: // 不需要处理
			break;
		case FIELDKEY_TURNOVER:
			if (1 == m_nZipOut)
			{
				sFldQuo += "TurnOver(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiTurnOver);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(dbValue / 1000.00 / 100.00);
				sFldQuo += ")|";
			}
		
			//以W为单位
			//stQuotation.m_dbTurnOver = dbValue / 1000 / 10000;
			stQuotation.m_uiTurnOver = static_cast<unsigned int>(dbValue / 1000.00 / 100.00);
			break;
		case FIELDKEY_AVERAGE:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "Average(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiAverage);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/10);
				sFldQuo += ")|";
			}

			stQuotation.m_uiAverage = uiValue/10;
			break;
		case FIELDKEY_SEQUENCENO:
			if (1 == m_nZipOut)
			{
			    sFldQuo += "SeqNo(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiSeqNo);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue/1000);
				sFldQuo += ")|";
			}

			if (stQuotation.m_uiSeqNo < uiValue/1000)
			{
				stQuotation.m_uiSeqNo = uiValue/1000;
			}
			else
			{
				CRLog(E_DEBUG,"(%s)旧的序列号：%u, 新序列号：%u", sInstID.c_str(),stQuotation.m_uiSeqNo, uiValue/1000);
				stQuotation.m_uiSeqNo = uiValue/1000;
				//nRtn = -1;
			}
			break;
		case FIELDKEY_QUOTETIME: //次字段处理需要再确认
			if (1 == m_nZipOut)
			{
				sFldQuo += "Time(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiTime);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue*1000);
				sFldQuo += ")|";
			}

			stQuotation.m_uiTime = uiValue*1000;
			//GetLocalTime(&sysTime);
			//uiTime = sysTime.wHour*10000000 + sysTime.wMinute*100000 + sysTime.wSecond*1000 + sysTime.wMilliseconds ;
			//stQuotation.m_uiTime = uiTime;
			break;
		case FIELDKEY_QUOTEDATE: //次字段处理需要再确认
			if (1 == m_nZipOut)
			{
				sFldQuo += "Date(";
				sFldQuo += ToString<unsigned int>(stQuotation.m_uiDate);
				sFldQuo += "-";
				sFldQuo += ToString<unsigned int>(uiValue);
				sFldQuo += ")|";
			}

			stQuotation.m_uiDate = uiValue;
			break;
		case FIELDKEY_UPDOWNRATE: // 不需要处理
			break;
		default:
			CRLog(E_ERROR,"行情报文字段序号[%d]不存在!",cField);
			break;
		}
		
	}

	if (1 == m_nZipOut)
	{
		CRLog(E_DEBUG, "%s %s", sInstID.c_str(), sFldQuo.c_str());
	}
	return nRtn;
}

//系统状态
//#ApiName=onSysStatChange#b_sys_stat=1#exch_date=20110616#m_sys_stat=1#sys_date=20110615#
void CTranslator::HandleSysStat(CBroadcastPacket& pkt)
{
	try
	{
		//由于本报文未入队列，用于进程初次运行时简单同步，保证初始全量行情先处理
		msleep(2);

		CCondMutexGuard oGuard(m_deqCondMutex);
		string sTmp;
		if (0 == pkt.GetParameterVal("exch_date", sTmp))
		{
			CGessDate oDate;
			oDate.FromString(sTmp);

			int nDiff = oDate.IntervalToToday();
			if (nDiff >= 0)
			{			
				CRLog(E_DEBUG, "onSysStatChange:%d", nDiff);

				unsigned int uiDate = FromString<unsigned int>(oDate.ToString());
				for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
				{
					(*it).second.m_uiDate = uiDate;
				}
			}
		}
	}
	catch(...)
	{
		CRLog(E_CRITICAL, "%s", "unknown exception");
	}
}

//合约状态转换
void CTranslator::HandleInstState(CBroadcastPacket& pkt)
{
	try
	{
		CCondMutexGuard oGuard(m_deqCondMutex);

		string sTmp;
		if (0 == pkt.GetParameterVal("tradeState", sTmp))
		{
			if (sTmp.length() > 0)
			{
				unsigned char ucInstStateOld = m_ucInstState;
				m_ucInstState = sTmp.at(0);

				if (I_END != ucInstStateOld && I_END == m_ucInstState
					|| I_END == ucInstStateOld && I_END != m_ucInstState)
				{
					for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
					{
						(*it).second.m_uiSeqNo = 0;
					}
				}

				if (I_NORMAL != ucInstStateOld && I_NORMAL == m_ucInstState)
				{					
					SYSTEMTIME sysTime;
					::GetLocalTime(&sysTime);
					unsigned int uiTime = sysTime.wHour*10000000 + sysTime.wMinute*100000 + sysTime.wSecond*1000 + sysTime.wMilliseconds ;					
					for (map<std::string, QUOTATION>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
					{
						(*it).second.m_uiTime = uiTime;
					}
				}
			}
		}
		CRLog(E_DEBUG, "TradeState=%c", m_ucInstState);
	}
	catch(...)
	{
		CRLog(E_CRITICAL, "%s", "unknown exception");
	}
}

//合约ID转换
void CTranslator::ConvertInstID(string& sInstID)
{
	if (sInstID == "Au99.99")
	{
		sInstID = "AU9999";
	}
	else if(sInstID == "Au99.95")
	{
		sInstID = "AU9995";
	}
	else if(sInstID == "Pt99.95")
	{
		sInstID = "PT9995";
	}
	else if(sInstID == "Au(T+5)")
	{
		sInstID = "AUT5";
	}
	else if(sInstID == "Au50g")
	{
		sInstID = "AU50";
	}
	else if(sInstID == "Au(T+D)")
	{
		sInstID = "AUTD";
	}
	else if(sInstID == "Ag99.9")
	{
		sInstID = "AG999";
	}
	else if(sInstID == "Ag(T+D)")
	{
		sInstID = "AGTD";
	}
	else if(sInstID == "Au100g")
	{
		sInstID = "AU100";
	}
	else if(sInstID == "Au(T+N1)")
	{
		sInstID = "AUTN1";
	}
	else if(sInstID == "Au(T+N2)")
	{
		sInstID = "AUTN2";
	}
	else if(sInstID == "Ag99.99")
	{
		sInstID = "AG9999";
	}
	else
	{
		if (sInstID.length() > 8)
		{
			sInstID = sInstID.substr(0,8);
		}
	}
}

//压缩报文调试输出命令
void CTranslator::ZipPktOut(int nOut)
{
	m_nZipOut = nOut;
	return;
}