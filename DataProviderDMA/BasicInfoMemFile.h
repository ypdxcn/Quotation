#pragma once
#include <map>
#include <string>
#include "osdepend.h"
#include "xmemmap.h"

using namespace std;


//市场类型 变量名:TMarketType
#define MARKET_SPOT			's'     //现货
#define MARKET_DEFER		'd'     //递延
#define MARKET_FUTURES		'f'     //期货
#define MARKET_FORWARD		'w'     //远期


//合约状态 变量名:TInstStateFlag
#define I_INITING		'0'     //初始化中
#define I_INIT		'1'     //初始化完成
#define I_BEGIN		'2'     //开盘
#define I_GRP_ORDER		'3'     //竞价报单
#define I_GRP_MATCH		'4'     //竞价撮合
#define I_NORMAL		'5'     //连续交易
#define I_PAUSE		'6'     //暂停
#define I_DERY_APP		'7'     //交割申报
#define I_DERY_MATCH		'8'     //交割申报结束
#define I_MID_APP		'9'     //中立仓申报
#define I_MID_MATCH		'A'     //交割申报撮合
#define I_END		'B'     //收盘


//市场状态 变量名:TMarketStateFlag
#define M_INITING		'0'     //初始化中
#define M_INIT		'1'     //初始化完成
#define M_OPEN		'2'     //开市
#define M_TRADE		'3'     //交易
#define M_PAUSE		'4'     //暂停
#define M_CLOSE		'5'     //收市


typedef struct
{
	char  instID[16];				//合约代码
	char  marketID;					//市场
	char  tradeState;				//合约交易状态
	unsigned int	uiSeqNo;
	unsigned int	uiClose;
	QUOTATION	stQuotation;
} INST_INFO,*PINST_INFO;


#define MAX_INST_NUMBER	32
typedef struct stBasicInf
{
	int nDate;
	int nCount;
	INST_INFO aInstState[MAX_INST_NUMBER];
} BASIC_INF;

//
class CBasicInfMemFile : public CXMemMapFile
{
public:
	CBasicInfMemFile();
	virtual ~CBasicInfMemFile(void);
	
public:
	BOOL Create(LPCTSTR lpszFileName);
	void Close();
	string ToString();

	BOOL GetInstState(const string& sInst, char& cState);
	BOOL SetInstState(const string& sInst, char cState, char cMarketID);
	int IsSeqNo(const string& sInst, unsigned int uiSeqNo,unsigned int uiClose);
	int SwitchTradeDate(unsigned int uiDate);
	unsigned int TradeDate();
private:
	BASIC_INF*					m_pBasicInf;
	map<string, INST_INFO*>		m_mapInstState;
private:

};
