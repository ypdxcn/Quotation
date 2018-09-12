#ifndef _MEM_DATE_H
#define _MEM_DATE_H
#include "Gess.h"
#include "Singleton.h"
#include "SubscriberTbl.h"
#include "QuotationTbl.h"

class CMemData : public CSingleton< CMemData >
{
	friend class CSingleton< CMemData >;
protected:
	CMemData();
	virtual ~CMemData();

public:
	//CQuotationTbl& GetQuotationTbl();

	///获取订阅管理
	CSubscriberTbl& GetSubscriberTbl();

	///获取合约管理
	//CContractMgr& GetContractMgr();

private:
	//CQuotationTbl  m_QuotationTbl;
	CSubscriberTbl m_SubscriberTbl;  //结点订阅管理器
};
#endif