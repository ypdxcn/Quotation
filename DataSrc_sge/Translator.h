#ifndef _TRANSLATOR_CP_H
#define _TRANSLATOR_CP_H
#include "BroadcastPacket.h"
#include "SamplerPacket.h"
#include "WorkThread.h"
#include "ConfigImpl.h"

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

// 日期定义
typedef struct tagDatePoint
{
    int year;
    int month;
    int day;
}DatePoint;

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

    // strTimeFilter格式: 12:00-15:59;21:00-23:59;00:00-5:59, 可以往后添加,
    // 12:00-15:59代表一个时间段, 用;号隔开每个时间段
    void set(string strTimeFilter)
    {
        slices.clear();

        vector<string> vTimeSlice = explodeQuoted(";", strTimeFilter);
        for (int i = 0; i < vTimeSlice.size(); i++)
        {
            vector<string> vTimePoint = explodeQuoted("-",vTimeSlice[i]);

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
                int nX = hour*100 + minute;
                int nBegin = ts.begin.hour*100 + ts.begin.minute;
                int nEnd = ts.end.hour*100 + ts.end.minute;
                if (nX >= nBegin && nX <= nEnd)
                {
                    bRet = true;
                    break;
                }
            }
        }

        return bRet;
    }
}TimeFilter;


// 数据过滤类
class DataFilter
{

public:
    DataFilter()
    {
        // 星期天的21:00至23:59点
        TimeSlice ts;
        ts.begin.hour = 21;
        ts.begin.minute = 0;
        ts.end.hour = 23;
        ts.end.minute = 59;
        timeFilters[6].slices.push_back(ts);

        // 星期一的00:00至05:59点
        ts.begin.hour = 0;
        ts.begin.minute = 0;
        ts.end.hour = 5;
        ts.end.minute = 59;
        timeFilters[0].slices.push_back(ts);
    }

public:
    vector<DatePoint> holidays;

    // 数据过滤时间片定义
    TimeFilter timeFilters[7];

    // strHoliday格式定义: 20120307,20120328,20120329
    void setHoliday(string strHoliday)
    {
        holidays.clear();

        vector<string> vHoliday = explodeQuoted(",", strHoliday);
        for (int i = 0; i < vHoliday.size(); i++)
        {
            if (vHoliday[i].length() == 8)
            {
                DatePoint dp;
                string strTmp;
                strTmp = vHoliday[i].substr(0, 4); 
                dp.year = atoi(strTmp.c_str());
                strTmp = vHoliday[i].substr(4, 2); 
                dp.month = atoi(strTmp.c_str());
                strTmp = vHoliday[i].substr(6, 2); 
                dp.day = atoi(strTmp.c_str());
                holidays.push_back(dp);
            }
        }
    }

    // dayOfWeek: 0为星期天，依此往下计数
    bool isFilter(int year, int month, int day, int dayOfWeek, int hour, int minute)
    {
        int i = (dayOfWeek+6)%7;

        if (isFilter(year, month, day))
        {
            return true;
        }

        return timeFilters[i].isFilter(hour, minute);
    }

    bool isFilter(int year, int month, int day)
    {
        bool bRet = false;

        if (!holidays.empty())
        {
            for (vector<DatePoint>::iterator it = holidays.begin(); 
                it != holidays.end(); it++)
            {
                DatePoint& dp = *it;
                if ((dp.day == day) && (dp.month == month) && (dp.year == year))
                {
                    bRet = true;
                    break;
                }
            }
        }

        return bRet;
    }
};
//

class CDataSrcCpMgr;
class CTranslator : public CConnectPointAsyn, public CWorkThread
{
public:
	CTranslator(void);
	~CTranslator(void);

	//基类CConnectPointAsyn的接口
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
	//基类CWorkThread线程接口
	int ThreadEntry();
	int End();

	int Translate(CBroadcastPacket& oPktSrc);
	int TranslateUnzipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	int TranslateZipPacket(CBroadcastPacket& oPktSrc, QUOTATION& stQuotation);
	void HandleInstState(CBroadcastPacket& pkt);
	void HandleSysStat(CBroadcastPacket& pkt);

	//合约ID转换
	void ConvertInstID(string& sInstID);

private:
	CDataSrcCpMgr*		m_pDataSrcCpMgr;
	unsigned long		m_ulKey;
	CConfig*			m_pCfg;

	CConfigImpl			m_oNameConvertFile;
	map<string, string>	m_mapNamePair;

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
    DataFilter m_dataFilter;
	
};
#endif