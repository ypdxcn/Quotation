#include <cassert>
#include "GessTime.h"
#include "PktBinBlockMon.h"
#include "NmCpMgr.h"
#include "NmService.h"


CNmService::CNmService(CNmCpMgr* pParent)
:m_pCfg(0)
,m_pParent(pParent)
,m_uiKeyPrefix(0)
{
	m_nState[0] = 0;
	m_nState[1] = 0;
	m_nState[2] = 0;
	
	memset(&m_stTsStatic[0], 0x00, sizeof(TS_STATICS));
	m_stTsStatic[0].uiDelayMin = 0xFFFFFFFF;
	memset(&m_stTsStatic[1], 0x00, sizeof(TS_STATICS));
	m_stTsStatic[1].uiDelayMin = 0xFFFFFFFF;
	memset(&m_stTsStatic[2], 0x00, sizeof(TS_STATICS));
	m_stTsStatic[2].uiDelayMin = 0xFFFFFFFF;
}

CNmService::~CNmService(void)
{
}

//初始化配置 
int CNmService::Init(CConfig* pCfg)
{
	unsigned int uiTimestamp = NowTimeStamp();
	uiTimestamp = uiTimestamp % 1000;
	unsigned int nValHi = uiTimestamp & 0xFFF;
	nValHi <<= 20;

	int RANGE_MIN = 0;
    int RANGE_MAX = 0xFFF;
	srand(static_cast<unsigned int>(time(0)));
	double dlRandNum = rand();
	double dlRadio = dlRandNum/RAND_MAX;
	double dlVal = dlRadio * (RANGE_MAX - RANGE_MIN) + RANGE_MIN;	
	unsigned int nValMi = static_cast<int>(dlVal);
	nValMi <<= 8;
	m_uiKeyPrefix = nValHi | nValMi;

	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;

	m_pCfg = pCfg;
	return 0;
}

//启动线程
int CNmService::Start()
{
	//启动调度线程
	BeginThread();
	return 0;
}

//停止线程
void CNmService::Stop()
{
	//停止调度线程
	CRLog(E_APPINFO,"%s","Stop CNmService Thread");
	EndThread();
}

//清理资源
void CNmService::Finish()
{
	m_vPkt.clear();
}

int CNmService::HandleMoncPkt(CBinBlockPkt& oPkt, unsigned long ulKey)
{
	TS_PKT ts;
	ts.ulKey = ulKey;
	//x86平台可以不用网络序
	ts.tsBody = *(TIMESTAMP_BODY*)(oPkt.Buffer());
	ts.tsBody.uicTickCount2 = NowTimeStamp();

	m_csPkt.Lock();
	m_vPkt.push_back(ts);
	m_csPkt.Unlock();
	return 0;
}

int CNmService::HandleMonsPkt(CBinBlockPkt& oPkt)
{
	TIMESTAMP_BODY stBody = *(TIMESTAMP_BODY*)(oPkt.Buffer());
	//x86平台可以不用网络序
	stBody.uisTime = NowTimeStamp();
	
	CBinBlockPkt oRsp(PKT_TS_RSP);
	oRsp.CopyKey(oPkt);
	oRsp.FillBuf((char*)(&stBody), sizeof(stBody));

	m_pParent->ToMons(oRsp);
	return 0;
}

int CNmService::HandleConnect(unsigned long ulKey, const string& sIp)
{
	if (ulKey > EnumKeyIfMonc3 || ulKey < EnumKeyIfMonc1)
		return -1;

	unsigned long ulIdx = ulKey - EnumKeyIfMonc1;
	m_csPkt.Lock();
	m_nState[ulIdx] = 1;
	strcpy(m_stTsStatic[ulIdx].sIp, sIp.c_str());
	m_csPkt.Unlock();
	return 0;
}

int CNmService::HandleClose(unsigned long ulKey)
{
	if (ulKey > EnumKeyIfMonc3 || ulKey < EnumKeyIfMonc1)
		return -1;

	unsigned long ulIdx = ulKey - EnumKeyIfMonc1;
	m_csPkt.Lock();
	m_nState[ulIdx] = 0;
	m_stTsStatic[ulIdx].uiCloseCount++;
	m_csPkt.Unlock();
	return 0;
}

int CNmService::ToMoncxReq(unsigned long ulKey)
{	
	CBinBlockPkt oReq(PKT_TS_REQ);
	unsigned int uiRouteKey = m_uiKeyPrefix | (ulKey - EnumKeyIfMonc1);
	oReq.RouteKey(uiRouteKey);

	TIMESTAMP_BODY stBody = {0};
	stBody.uicTickCount1 = NowTimeStamp();
	

	oReq.FillBuf((char*)(&stBody), sizeof(stBody));
	m_pParent->ToMoncx(oReq, ulKey);
	return 0;
}

int CNmService::ThreadEntry()
{
	try
	{
		msleep(3);

		unsigned int uiCount = 0;
        while(!m_bEndThread)
		{
			m_csPkt.Lock();
			for (vector<TS_PKT>::iterator it = m_vPkt.begin(); it != m_vPkt.end(); ++it)
			{
				TS_PKT& stPkt = *it;
				if (stPkt.ulKey > EnumKeyIfMonc3 || stPkt.ulKey < EnumKeyIfMonc1)
					continue;

				int nIdx = stPkt.ulKey - EnumKeyIfMonc1;
				unsigned int uiTm1 = stPkt.tsBody.uicTickCount1/1000;
				unsigned int uimSec1 = stPkt.tsBody.uicTickCount1 % 1000;
				unsigned int uiTm2 = stPkt.tsBody.uicTickCount2/1000;
				unsigned int uimSec2 = stPkt.tsBody.uicTickCount2 % 1000;
				CGessTime oTime1(uiTm1);
				CGessTime oTime2(uiTm2);
				int nDiff = oTime2 - oTime1;
				if (nDiff < 0)
					continue;

				unsigned int uiDelay = 1000*nDiff + (uimSec2 - uimSec1);
				if (m_stTsStatic[nIdx].uiDelayCount > 0 && m_stTsStatic[nIdx].uiDelayCount % 120 == 0)
				{
					unsigned int uimSecondLcl = NowMSec();
					unsigned int uimSecondRmt = MSecFromTimestamp(stPkt.tsBody.uisTime);
					m_stTsStatic[nIdx].nTimeDiff = uimSecondRmt + m_stTsStatic[nIdx].uiDelayAvg - uimSecondLcl;
				}
				
				m_stTsStatic[nIdx].uiDelayTotal += uiDelay;
				m_stTsStatic[nIdx].uiDelayCount++;
				m_stTsStatic[nIdx].uiDelayAvg = m_stTsStatic[nIdx].uiDelayTotal/m_stTsStatic[nIdx].uiDelayCount;
				
				if (m_stTsStatic[nIdx].uiDelayMax < uiDelay)
				{
					m_stTsStatic[nIdx].uiDelayMax = uiDelay;
				}

				if (m_stTsStatic[nIdx].uiDelayMin > uiDelay)
				{
					m_stTsStatic[nIdx].uiDelayMin = uiDelay;
				}
			}
			m_vPkt.clear();

			int nIdx = uiCount % 3;
			int nState = m_nState[nIdx];
			if (1 == nState)
			{
				ToMoncxReq(EnumKeyIfMonc1 + nIdx);
			}

			uiCount++;
			if (uiCount % 120 == 0)
			{
				for (int i = 0; i < 3; i++)
				{
					CRLog(E_STATICS, "%d:close(%u), delay min-max-avg(%05u-%05u-%05u)ms, timediff(%04d.%03d)s.ms, %s", i, m_stTsStatic[i].uiCloseCount, m_stTsStatic[i].uiDelayMin, m_stTsStatic[i].uiDelayMax, m_stTsStatic[i].uiDelayAvg, m_stTsStatic[i].nTimeDiff/1000, m_stTsStatic[i].nTimeDiff % 1000, m_stTsStatic[i].sIp); 
					m_stTsStatic[i].uiDelayCount = 0;
					m_stTsStatic[i].uiDelayTotal = 0;
					m_stTsStatic[i].uiDelayMin = 0xFFFFFFFF;
					m_stTsStatic[i].uiDelayMax = 0;
				}
			}
			m_csPkt.Unlock();

			msleep(1);
		}
		CRLog(E_APPINFO,"%s","CNmService Thread exit!");
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

int CNmService::End()
{
	CRLog(E_APPINFO,"%s","CNmService thread wait end");
	Wait();
	return 0;
}

unsigned int CNmService::NowTimeStamp()
{
	unsigned int uiTimeStamp = 0;
#ifdef WIN32
	SYSTEMTIME st;
	::GetLocalTime(&st);
	uiTimeStamp = st.wHour*10000000 + st.wMinute * 100000 + st.wSecond * 1000 + st.wMilliseconds;
#else
	struct timeval tv;
    gettimeofday(&tv,0);

	time_t rawtime = tv.tv_sec;
	struct tm * timeinfo = localtime (&rawtime);
	uiTimeStamp = timeinfo->tm_hour*10000000 + timeinfo->tm_min * 100000 + timeinfo->tm_sec * 1000 + tv.tv_usec/1000;
#endif
	return uiTimeStamp;
}

unsigned int CNmService::NowMSec()
{
	unsigned int uiTimeStamp = 0;
#ifdef WIN32
	SYSTEMTIME st;
	::GetLocalTime(&st);
	uiTimeStamp = st.wHour*3600*1000 + st.wMinute*60*1000 + st.wSecond*1000 + st.wMilliseconds;
#else
	struct timeval tv;
    gettimeofday(&tv,0);

	time_t rawtime = tv.tv_sec;
	struct tm * timeinfo = localtime (&rawtime);
	uiTimeStamp = timeinfo->tm_hour*3600*1000 + timeinfo->tm_min*60*1000 + timeinfo->tm_sec*1000 + tv.tv_usec/1000;
#endif
	return uiTimeStamp;
}

unsigned int CNmService::MSecFromTimestamp(unsigned int uiTimeStamp)
{
	unsigned int uiHour = uiTimeStamp / 10000000;
	unsigned int uiMinute = (uiTimeStamp % 10000000) / 100000;
	unsigned int uiSecond = (uiTimeStamp % 100000) / 1000;
	unsigned int uiMillSecond = uiTimeStamp % 1000;
	unsigned int uimSecond = uiHour*3600*1000 + uiMinute * 60 * 1000 + uiSecond * 1000 + uiMillSecond;
	return uimSecond;
}