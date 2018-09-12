#ifndef _DELIVER_CP_H
#define _DELIVER_CP_H
#include "Comm.h"
#include "workthread.h"
#include "GessDate.h"
#include "GessTime.h"
#include "SamplerPacket.h"
#include "MemData.h"
#include "NetMgr.h"

#define MAX_UINT (0xFFFFFFFF)

// 增加时间定义, Jerry Lee, 2012-3-22

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

class CQuoSvrCpMgr;
class CMemData;
class CDeliverMgr :public CConnectPointAsyn, public CWorkThread
{
	class CQuoSvrMgrModule: public CNmoModule
	{
	public:
		CQuoSvrMgrModule():m_pParent(0){}
		virtual ~CQuoSvrMgrModule(){}
		void Bind(CDeliverMgr* p){m_pParent = p;}
		//网管单个查询接口
		int Query(CNMO& oNmo) const
		{
			if (0 == m_pParent)
				return -1;

			return m_pParent->Query(oNmo);
		}
		//批量查询接口
		int Query(vector< CNMO > & vNmo) const
		{
			for (vector< CNMO >::iterator it = vNmo.begin(); it != vNmo.end(); ++it)
			{
				Query(*it);
			}
			return 0;
		}
		//控制接口
		int Operate(const string &sOid, int nflag, const string &sDstValue, const string &sParamer) {return -1;}
	private:
		CDeliverMgr* m_pParent;
	};
public:
	CDeliverMgr(void);
	~CDeliverMgr(void);

	int Init(CConfig* pConfig);
	int Start();
	void Stop();
	void Finish();
	int OnRecvPacket(CPacket &GessPacket){return 0;}
	int SendPacket(CPacket &pkt);
	void Bind(CConnectPointManager* pCpMgr,const unsigned long& ulKey);
	
	// added by Ben, 2011-04-01, 转发历史、资讯、实时包
	int HandleHistoryData(CSamplerPacket& oPkt);

	// added by Jerry Lee, 2010-12-23, 发送数据包给客户端
    void HandleHistoryData(CPacket &pkt, unsigned int uiNodeId);

    // added by Jerry Lee, 2011-2-25, 发送资讯给客户端
    void HandleInfoData(CPacket &pkt, unsigned int uiNodeId);

private:
	CQuoSvrCpMgr*     m_pQuoSvrCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;
	CQuoSvrMgrModule	m_oMgrModule;

	//统计带宽	
	unsigned int m_uiFwdCount;				//累计接收包数
	unsigned int m_uiFwdGBytes;				//累计发送M字节数
	unsigned int m_uiFwdBytes;				//累计发送字节数
	unsigned int m_uiFwdGBytesLast;			//上次计算时M字节数
	unsigned int m_uiFwdBytesLast;			//上次计算时字节数
	CGessDate		m_oRecDate;				//上次计算时日期
	CGessTime		m_oRecTime;				//上次计算时时间
	int				m_nInterval;			//累计秒数
	unsigned int	m_uiLastBandWidth;		//最大统计带宽
	unsigned int	m_uiMaxBandWidth;		//最大统计带宽
	unsigned int	m_uiMinBandWidth;		//最小统计带宽
	unsigned int	m_uiAvgBandWidth;		//平均统计带宽
	
	unsigned int m_uiQuoPktCount;			//累计行情包数
	unsigned int m_uiQuoPktBytes;			//累计行情包字节数
	unsigned int m_uiQuoPktGBytes;			//累计发送M字节数
	unsigned int m_uiOrderFwdCount;			//订阅触发发送包数
	unsigned int m_uiOrderFwdBytes;			//订阅触发发送字节数

	std::deque<CSamplerPacket> m_deqQuotation;

	typedef multimap<string,unsigned int> MMAP_ORDER;
	typedef MMAP_ORDER::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_ORDER;
	MMAP_ORDER m_mmapQuotationOrder;		//第一次订阅需要发送完整行情的合约

	CCondMutex	m_deqCondMutex;

    TimeFilter m_timeFilters[7];

	int ThreadEntry();
	int End();

	int HandlePacket(CSamplerPacket& oPkt);
	int HandleOrder(CSamplerPacket& oPkt);
	int HandleCancelOrder(CSamplerPacket& oPkt);
	int HandleQuotationDlv(CSamplerPacket& oPkt);
	int HandleFirstQuotation(string sKey,unsigned int uiNodeID);
	int AssemblePkt(QUOTATION& stQuotation, string& sQuotationVal, bitset<FIELDKEY_UNKNOWN>& bsQuotation);
	int GetInstID(CSamplerPacket& oPkt, map<string,string>& mapInstID);
	bool Statics(string& sQuotationVal);
	int Query(CNMO& oNmo);

	///根据结点订阅信息判断当前合约是否需要通知，0--需要通知，非0-无需通知
	int CheckSendMsgRole(SUB_CONTEXT& SubscriberContext,const unsigned int & musMarketType);

};
#endif