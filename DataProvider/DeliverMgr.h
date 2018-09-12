#ifndef _DELIVER_CP_H
#define _DELIVER_CP_H
#include "Comm.h"
#include "workthread.h"
#include "GessDate.h"
#include "GessTime.h"
#include "ConfigImpl.h"
#include "SamplerPacket.h"
#include "MemData.h"
#include "NetMgr.h"

class CProviderCpMgr;
class CMemData;
class CDeliverMgr :public CConnectPointAsyn, public CWorkThread
{
private:
	int Query(CNMO& oNmo) ;
	class CDeliverMgrNm: public CNmoModule
	{
	public:
		CDeliverMgrNm():m_pParent(0){}
		virtual ~CDeliverMgrNm(){}
		void Bind(CDeliverMgr* pParent){m_pParent = pParent;}
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
		CDeliverMgr * m_pParent;
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
	
	///行情信息入队列
	int Enque(QUOTATION& stQuotation);
	string HandleCmdLine(const string& sCmd, const vector<string>& vecPara);
private:
	CProviderCpMgr*     m_pProviderCpMgr;
	unsigned long	m_ulKey;
	CConfig*		m_pCfg;
	CConfigImpl		m_oUnitFile;
	map<string, unsigned int>	m_mapUnit;

	//
	CDeliverMgrNm	m_oNmoModule;

	//储存最新完整黄金行情,用于增量比较
	map<std::string, QUOTATION> m_mapQuotation;
	string			m_sFileLast;

	//统计带宽
	unsigned int m_uiFwdCount;				//累计发送包数
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

	unsigned char   m_ucMaxPerPkt;			//每个报文最大行情数
	//std::deque<CSamplerPacket> m_deqQuotation;
	std::deque<QUOTATION> m_deqQuotation;
	//CSamplerPacket格式订阅报文
	std::deque<CSamplerPacket> m_deqOrder; 

	typedef multimap<string,unsigned int> MMAP_ORDER;
	typedef MMAP_ORDER::iterator MMAP_IT;
	typedef pair<MMAP_IT,MMAP_IT> RANGE_ORDER;
	MMAP_ORDER m_mmapQuotationOrder;		//第一次订阅需要发送完整行情的合约

	CCondMutex	m_deqCondMutex;

	int ThreadEntry();
	int End();

	int HandleQuotationOrder(CSamplerPacket& oOrder);
	int HandleOrder(CSamplerPacket& oOrder);
	int HandleCancelOrder(CSamplerPacket& pkt);
	int HandleFirstQuotation(const string& sKey,unsigned int uiNodeID);
	int AssemblePkt(QUOTATION& stQuotation, string& sQuotationVal, bitset<FIELDKEY_UNKNOWN>& bsQuotation);
	int FindDifference(QUOTATION& stQuotation, bitset<FIELDKEY_UNKNOWN>& bsQuotation);
	int HandleQuotationDlv(QUOTATION& stQuotation, unsigned char& cPkt, string& sQuotationVal,unsigned int& uiLastMarketType);
	int DelieveQuotation(unsigned int uiMarketType, string& sQuotationVal);
	bool Statics(string& sQuotationVal);

	///根据结点订阅信息判断当前合约是否需要通知，0--需要通知，非0-无需通知
	int CheckSendMsgRole(SUB_CONTEXT& SubscriberContext,const unsigned int & musMarketType);

	//
	string HandleCmdLineReplay(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineBuffer(const string& sCmd, const vector<string>& vecPara);
	string OnCmdLineLoad(const string& sCmd, const vector<string>& vecPara);
};
#endif