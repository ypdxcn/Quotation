#include "Market.h"
#include <iostream>
#include "strutils.h"

CMarketMgr::CMarketMgr(void)
{
	m_pConfig = new CConfigImpl();

}


CMarketMgr::~CMarketMgr(void)
{

}
int CMarketMgr::Init()
{
	string sCfgFile = MARKET_MGR_CFG;

	cout << "加载配置文件..." << endl;

	std::string sCfgFilename;
	sCfgFilename = DEFUALT_CONF_PATH PATH_SLASH;
	sCfgFilename = sCfgFilename + sCfgFile;

	if (m_pConfig->Load(sCfgFilename) != 0)
	{
		cout << "加载配置文件[" << sCfgFilename << "]失败!" << endl;
		msleep(3);
		return -1;
	}

	// 取得市场分类数组
	string sTmp = "";
	if (0 == m_pConfig->GetProperty(CFG_MARKET_CLASS, sTmp))
	{
		cout << "加载配置文件项【" << CFG_MARKET_CLASS << "】失败!" << endl;
		msleep(3);
		return -1;
	}

	// 根据分类数组逐个初始化市场对象
	vector<string> vMarketTypes = explodeQuoted(",",sTmp);
	for (int i = 0; i < vMarketTypes.size(); i ++)
	{
		CConfig *pCfgMarket;
		pCfgMarket = m_pConfig->GetProperties(vMarketTypes[i]);
		if (!pCfgMarket)
		{
			cout << "加载配置文件项【" << vMarketTypes[i] << "】失败!" << endl;
			msleep(3);
			continue;
		}

		HSMarketDataType marketType = vMarketTypes[i];
		CMarket* pMarket = new CMarket();
		if (!pMarket)
		{
			cout << "new CMarket 失败!" << endl;
			msleep(3);
			return -1;
		}
		//pMarket->Init(pCfgMarket);
		pMarket->Init(pCfgMarket);//marketType
		m_mapMarkets[marketType] = pMarket;
	}
}


#define CFG_MARKET_CLASS                  "市场分类"
#define CFG_MARKET_SECTION_TYPE           "市场类型"
#define CFG_MARKET_SECTION_NAME           "市场名称"
#define CFG_MARKET_SECTION_TRADEPHASE     "交易时间"
#define CFG_MARKET_SECTION_PERHAND        "每手单位"
#define CFG_MARKET_SECTION_EXTENDMUTILE   "放大倍数"

int CMarket::Init(CConfig* pCfg)
{
	if (m_bInit)
		return E_ALREADDONE;

	assert(0 != pCfg);
	if (0 == pCfg)
		return -1;
	m_pCfg = pCfg;

	if (0 == m_pCfg->GetProperty(CFG_MARKET_SECTION_TYPE, sTmp))
	{
		cout << "加载配置文件项【" << CFG_MARKET_SECTION_TYPE << "】失败!" << endl;
		msleep(3);
		return -1;
	}

	HSMarketDataType marketType;
	stringstream sStream(sTmp);
	sStream >> hex >> marketType;
	cout << marketType << endl;

	// 加载硬盘文件
	if (LoadFromDisk(marketType))
	{
		return true;
	}

	QMarketInfo stMarkeInfo;
	QTradePhase* pMarketTradePhases = NULL;
	memset(&stMarkeInfo, 0, sizeof(QMarketInfo));
	stMarkeInfo.m_MarketType    = marketType;

	if (m_pCfg->GetProperty(CFG_MARKET_SECTION_NAME, sTmp))
	{
		if (sTmp.size() <= MARKETNAME_SIZE)
			strcpy(stMarkeInfo.m_acMarketName, sTmp.c_str()); 
		else
			strncpy(stMarkeInfo.m_acMarketName, sTmp.c_str(), MARKETNAME_SIZE); 
	}

	// 取得市场的交易时间段
	if (m_pCfg->GetProperty(CFG_MARKET_SECTION_TRADEPHASE, sTmp))
	{
		vector<string> vTradePhases = explodeQuoted(",",sTmp);
		stMarkeInfo.m_sPhasesCount = vTradePhases.size();
		pMarketTradePhases = new QTradePhase[vTradePhases.size()];
		if (!pMarketTradePhases)
		{
			char acErr[256];
			sprintf(acErr, "申请内存 %s! 【new QTradePhase[%d]】失败!", stMarkeInfo.m_acMarketName, vTradePhases.size());
			CRLog(E_CRITICAL, acErr);
			exit(1);
		}

		for (int i = 0; i < vTradePhases.size(); i ++)
		{
			vector<string> vPhase = explodeQuoted("-", vTradePhases[i]);
			if (vPhase.size() < 2)
			{
				char acErr[256];
				sprintf(acErr, "交易时间配置错误! %s", stMarkeInfo.m_acMarketName);
				CRLog(E_CRITICAL, acErr);
				exit(1);
			}
			pMarketTradePhases[i].m_nBeginTime = atoi(vPhase[0]);
			pMarketTradePhases[i].m_nEndTime = atoi(vPhase[1]);
		}
		stMarkeInfo.m_pTradePhases = pMarketTradePhases;
	}

	// 取得市场板块分类表
	if (m_pCfg->GetProperty(CFG_MARKET_SECTION_SORT, sTmp))
	{
		vector<string> vSorts = explodeQuoted(",",sTmp);
		stMarkeInfo.m_sSortCount = vSorts.size();
		QStockSort* pSort = new QStockSort[vSorts.size()];
		if (!pSort)
		{
			char acErr[256];
			sprintf(acErr, "申请内存 %s! 【new QStockSort[%d]】失败!", stMarkeInfo.m_acMarketName, vSorts.size());
			CRLog(E_CRITICAL, acErr);
			exit(1);
		}

		for (int i = 0; i < vTradePhases.size(); i ++)
		{
			vector<string> vPhase = explodeQuoted("-", vTradePhases[i]);
			if (vPhase.size() < 2)
			{
				char acErr[256];
				sprintf(acErr, "交易时间配置错误! %s", stMarkeInfo.m_acMarketName);
				CRLog(E_CRITICAL, acErr);
				exit(1);
			}
			pMarketTradePhases[i].m_nBeginTime = atoi(vPhase[0]);
			pMarketTradePhases[i].m_nEndTime = atoi(vPhase[1]);
		}
		stMarkeInfo.m_pTradePhases = pMarketTradePhases;
	}


	m_Buffer_Market.Alloc();

	// 通过配置做初始化

	// 取得交易时间段

	// 取得品种数量

	// 取得板块分类表

	string szSection;
	sprintf(szSection, "%s.CFG_MARKET_SECTION_NAME", vMarketTypes[i]);
	if (0 == m_pConfig->GetProperty(szSection, sTmp))
	{
		cout << "加载配置文件项【" << szSection << "】失败!" << endl;
		msleep(3);
		return -1;
	}

	stringstream sStream(vMarketTypes[i]);
	sStream >> hex >> marketType;
	cout << marketType << endl;


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
