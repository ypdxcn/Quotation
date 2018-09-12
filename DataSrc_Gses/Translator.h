#ifndef _TRANSLATOR_CP_H
#define _TRANSLATOR_CP_H
#include "BroadcastPacket.h"
#include "SamplerPacket.h"
#include "WorkThread.h"

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


// 增加时间定义, Jerry Lee, 2012-3-26

// 时间点定义
typedef struct tagTimePoint
{
    int hour;
    int minute;
}TimePoint;

// 时间片定义
typedef struct tagTimeSlice
{
    TimePoint begin; // 开始时间
    TimePoint end;   // 结束时间
}TimeSlice;

// 过滤时间段定义
typedef struct tagTimeFilter
{
    vector<TimeSlice> slices;

    void set(string strTimeFilter)
    {
        slices.clear();

        vector<string> vTimeSlice = explodeQuoted(";", strTimeFilter);
        for (int i = 0; i < vTimeSlice.size(); i++)
        {
            vector<string> vTimePoint = explodeQuoted(",",vTimeSlice[i]);

            TimeSlice ts;
            if (vTimePoint.size() == 2)
            {
                vector<string> vBegin = explodeQuoted(":", vTimePoint[0]);
                vector<string> vEnd = explodeQuoted(":", vTimePoint[1]);

                if ((vBegin.size() == 2) && (vEnd.size() == 2))
                {
                    ts.begin.hour = strutils::FromString<int>(vBegin[0]);
                    ts.begin.minute = strutils::FromString<int>(vBegin[1]);

                    ts.end.hour = strutils::FromString<int>(vEnd[0]);
                    ts.end.minute = strutils::FromString<int>(vEnd[1]);

                    slices.push_back(ts);
                }
            }
        }
    }

    bool isFilter(int hour, int minute)
    {
        bool bRet = false;

        if (!slices.empty())
        {
            for (vector<TimeSlice>::iterator it = slices.begin(); 
                it != slices.end(); it++)
            {
                TimeSlice& ts = *it;
                if (((hour >= ts.begin.hour) && (minute >= ts.begin.minute))
                    && ((hour <= ts.end.hour) && (minute <= ts.end.minute)))
                {
                    bRet = true;
                    break;
                }
            }
        }

        return bRet;
    }
}TimeFilter;

class CDataSrcCpMgr;
class CTranslator : public CConnectPointAsyn, public CWorkThread
{
public:
	CTranslator(void);
	~CTranslator(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);
	unsigned int QueueLen() {return m_deqQuotation.size();}	
	unsigned int InPktStatic() {return m_uiInCount;}
	void ZipPktOut(int nOut);
private:
	CDataSrcCpMgr*     m_pDataSrcCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;

	//市场合约状态
	unsigned char	m_ucInstState;
	//接收包数量
	unsigned int	m_uiInCount;	
	//压缩报文调试输出
	int				m_nZipOut;

	//行情广播报文(全量或增量)
	std::deque<CBroadcastPacket> m_deqQuotation;
	CCondMutex	m_deqCondMutex;
	
	//储存最新完整黄金行情,用于发送全量行情
	map<std::string, QUOTATION> m_mapQuotation;

    // 数据过滤时间片定义
    TimeFilter m_timeFilters[7];

	int ThreadEntry();
	int End();

	int Translate(CBroadcastPacket& oPktSrc);
	int TranslateUnzipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	int TranslateZipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	void HandleInstState(CBroadcastPacket& pkt);
	void HandleSysStat(CBroadcastPacket& pkt);

	//合约ID转换
	void ConvertInstID(string& sInstID);
};
#endif