#ifndef _MARKET_H
#define _MARKET_H

#include "stdef.h"
#include "Comm.h"

#include "configImpl.h"

#define E_ALREADDONE      0
#define E_SUCCEED         1
#define E_FAIL           -1

#define MARKET_MGR_CFG   "markets.cfg"

#define CFG_MARKET_CLASS                  "市场分类"
#define CFG_MARKET_SECTION_NAME           "市场名称"
#define CFG_MARKET_SECTION_SORT           "板块分类"
#define CFG_MARKET_SECTION_TRADEPHASE     "交易时间"
#define CFG_MARKET_SECTION_PERHAND        "每手单位"
#define CFG_MARKET_SECTION_EXTENDMUTILE   "放大倍数"


class CDataBuffer;
class CMTDataBuffer;
class CMarket
{
public:
	CMarket();
	~CMarket();
	int Init(HSMarketDataType marketType, string szMarketName);  //初始化，根据类型和名称从本地文件加载市场信息
	int Init(CConfig* pCfg);
	int Open();
	int Close();

	void OnAsk(const char* pszBuf, const int nSize, CDataBuffer& outBuffer);
	void OnRecv(const char* pszBuf, const int nSize);

	QMarketInfo* GetMarketInfo();

	CMTDataBuffer* GetMarketBuffer(){return m_Buffer_Market;}

private:
	CMTDataBuffer   *m_Buffer_Market;
	string          m_szFile;
	CConfig*	    m_pCfg;	
	bool            m_bInit;

private:
	void LoadFromDisk(HSMarketDataType marketType);
	void WriteToFile();
};

class CMarketMgr
{
public:
	// 事件处理定时器
	class CEventTimer : public CGessTimer
	{
	public:
		CEventTimer():m_pParent(0){}
		virtual ~CEventTimer(){}
		void Bind(CMarketMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey, unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnEventTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CMarketMgr* m_pParent;
	};

public:
	 CMarketMgr(void);
	 ~CMarketMgr(void);

	int Init();   // 初始化，加载m_mapMarkets

	void OnAsk(const char* pszBuf, const int nSize, CDataBuffer& outBuffer);
	void OnRecv(const char* pszBuf, const int nSize);

	CMarket* GetMarket(ULONG lMarketType);           // 根据市场类型取得市场指针
	CMarket* GetMarket(const char * pszCode);        // 根据品种代码取得市场指针

	CEventTimer m_oEventTimer;

	int OnEventTimeout(const string& ulKey, unsigned long& ulTmSpan);
private:
	map<ULONG, CMarket*> m_mapMarkets;
	CConfigImpl*    m_pConfig;

};



#endif