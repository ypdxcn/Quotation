#ifndef _DATA_SRC_CP_MGR_H
#define _DATA_SRC_CP_MGR_H
#include "netlogdev.h"
#include "IpcPacket.h"
#include "Comm.h"
#include <iostream>
#include "WorkThreadNm.h"
#include "WorkThread.h"
#include "Logger.h"
#include "SamplerPacket.h"
#include "XQueueIo.h"
#include "MemShareAlive.h"


using namespace std;


//连接点key常量
typedef enum tagEnumKey
{
	EnumKeyIfH1,
	EnumKeyIfH2,
	EnumKeyIfMC,
	EnumKeyIfMS,
	EnumKeyIfCmd,
	EnumKeyCmdHandler,
	EnumNetMagModule,
	EnumKeyTraderLoginMgr,
	EnumKeyUnknown
} EnumKeyIf;

//各个连接点配置项名
const string gc_sCfgIfH1 = "IFH1";
const string gc_sCfgIfH2 = "IFH2";
const string gc_sCfgIfCmd = "IFCMD";
const string gc_sCfgIfMC = "IFMC";
const string gc_sCfgIfMS = "IFMS";
const string gc_sCfgNetMagModule = "net_mgr";
const string gc_sCfgTraderLoginMgr = "login_mgr";

//缺省命令字匹配
const string gc_sDefaultCmdID = "#";


class CConfigImpl;
class CXQueueWriter;

class CGessTimerMgr;
class CConfigImpl;
class COfferConnPoint;
class CLoginMgr;
class CNetMgrModule;

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

class CDataSrcCpMgr:public CProtocolCpMgr
{
	//心跳定时器
	class CDogTimer : public CGessTimer
	{
	public:
		CDogTimer():m_pParent(0){}
		virtual ~CDogTimer(){}
		void Bind(CDataSrcCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& ulKey,unsigned long& ulTmSpan)
		{
			if (0 != m_pParent)
				return m_pParent->OnDogTimeout(ulKey,ulTmSpan);
			return -1;
		}
		void TimerCanceled(const string& ulKey)	{}
	private:
		CDataSrcCpMgr* m_pParent;
	};

	//定时重启定时器
	class CResetTimer: public CGessAbsTimer
	{
	public:
		CResetTimer():m_pParent(0){}
		~CResetTimer(void){}
		void Bind(CDataSrcCpMgr* p) {m_pParent=p;}
		int TimeOut(const string& sKey)
		{
			if (0 != m_pParent)
				return m_pParent->OnResetTimeout(sKey);
			return -1;
		}
		void TimerCanceled(const string& ulKey){}
	private:
		CDataSrcCpMgr* m_pParent;
	};
private:
	int Query(CNMO& oNmo) ;
	class CDataSrcCpMgrNm: public CNmoModule
	{
	public:
		CDataSrcCpMgrNm():m_pParent(0){}
		virtual ~CDataSrcCpMgrNm(){}
		void Bind(CDataSrcCpMgr* pParent){m_pParent = pParent;}
		//单个查询接口
		int Query(CNMO& oNmo) const
		{
			if (0 != m_pParent)
				return m_pParent->Query(oNmo);
			return -1;
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
		CDataSrcCpMgr * m_pParent;
	};
public:
	CDataSrcCpMgr();
	virtual ~CDataSrcCpMgr();

	int Init(const string& sProcName);
	void Finish();
	int Start();
	void Stop();

	int OnRecvQuotation(QUOTATION& stQuotation, int nDelay);

	bool Stock_Dll_Init();
	bool Stock_Dll_Release();


public:
	int OnConnect(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	int OnAccept(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);	
	int OnClose(const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort);
	int OnLogin( const unsigned long& ulKey,const string& sLocalIp, int nLocalPort, const string& sPeerIp, int nPeerPort,int nFlag);
	//int OnLogout
	int Forward(CPacket &GessPacket,const unsigned long& ulKey);

	int Run();
	void StopMe();
	int StartMe();

	int Bind(long ulHwnd);

	bool Logout();

	int InitRouterTbl();

private:
	int OnResetTimeout(const string& sTmKey);
	int OnDogTimeout(const string& sTmKey,unsigned long& ulTmSpan);
	
	//网管事件通知
	void NotifyEvent(const string& sEvt,int nGrade = 1);	

private:

	//定义成员函数指针
	typedef string (CDataSrcCpMgr::*MFP_CmdHandleApi)(const string& sCmd, const vector<string>& vecPara);
	//报文命令字与报文处理成员函数映射结构
	typedef struct tagCmdLine2Api
	{
		string sCmdName;					//CmdName
		string sCmdAbbr;					//命令缩写
		MFP_CmdHandleApi pMemberFunc;		//报文处理函数指针
		string sHelp;						//命令说明
	} CmdLine2Api;
	//命令行命令字与命令处理成员函数映射表
	static CmdLine2Api m_CmdLine2Api[];


	//源接口+报文命令字与对应路由接口映射结构
	typedef struct tagIfRouterCfg
	{
		CConnectPointAsyn* pCp;
		unsigned long ulIfTo;
		unsigned long ulIfFrom;
		string sCmdID;
	} IfRouterCfg;
	//源接口+报文命令字与对应路由接口映射关系配置表
	static IfRouterCfg m_tblIfRouterCfg[];

	//路由点
	typedef multimap<string,CConnectPointAsyn*> MMAP_CP;
	typedef MMAP_CP::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_CP;
	typedef struct tagIfRouterPoint
	{
		unsigned long ulIfFrom;
		MMAP_CP  mmapCmds;
	} IfRouterPoint;
	//内存路由表
	IfRouterPoint m_tblIfRouter[EnumKeyUnknown];

private:
	CConnectPointAsyn*		m_pCpInterfaceMC;		//报盘机主备M客户端接口
	CConnectPointAsyn*		m_pCpInterfaceMS;		//报盘机主备M服务端接口

	CLoginMgr*				m_pOfferLoginMgr;		//主备登录管理


	CConnectPointAsyn*		m_pCpInterfaceH1;		//系统监控H1接口
	CConnectPointAsyn*		m_pCpInterfaceH2;		//系统监控H2接口
	CNetMgrModule*			m_pNetMagModule;		//网管代理


	//看门狗共享内存
	CMemShareAlive			m_oMemShareAlive;

	//
	CDataSrcCpMgrNm			m_oNmoModule;

	//dog定时器
	CDogTimer m_oIfkTimer;
	//定时重启定时器
	CResetTimer m_oResetTimer;
	//定时重启时间点配置
	vector<CGessTime> m_vResetTime;	


	unsigned int		  m_uiNodeID;
	unsigned int		  m_uiNodeType;
	CConfigImpl*		m_pConfig;
	volatile bool m_bFwdStop;

	long m_hwndOwner;
	string m_sWjfName;

    // 数据过滤时间片定义
    DataFilter m_dataFilter;

public:
	string& GetWjfName(){return m_sWjfName;}

private:
	vector< CXQueueIo<QUOTATION>* >			m_vecQueueIo;          //        //
	
	//转发数量
	unsigned int  m_uiFwdTotal;

	//延迟统计信息
	unsigned int  m_uiDelayLess0s;
	unsigned int  m_uiDelayLess1s;
	unsigned int  m_uiDelayLess2s;
	unsigned int  m_uiDelayLess3s;
	unsigned int  m_uiDelayLess5s;
	unsigned int  m_uiDelayLess10s;
	unsigned int  m_uiDelayLess30s;
	unsigned int  m_uiDelayLess60s;
	unsigned int  m_uiDelayLess120s;
	unsigned int  m_uiDelayMore120s;
	unsigned int  m_uiDelayMin;
	unsigned int  m_uiDelayMax;
};
#endif