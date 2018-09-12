/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	InfoPublisher.h 
* Programer(s):	Jerry Lee 
* Created:      20110214 
* Description:	interface for publishing information
* History:
*******************************************************************************/

#pragma once

#define  _AFXDLL

#ifdef _WINDOWS_
#undef _WINDOWS_
#endif

#define BOOST_DATE_TIME_SOURCE

#include <afxdb.h>
#include "SamplerPacket.h"
#include "DeliverMgr.h"
#include <string>
#include <odbcinst.h>

using namespace std;

#pragma warning (disable:4005 4018)

//
class CInfoPublisher
{
public:
    CInfoPublisher();
    ~CInfoPublisher();

public:
   /**
	* 开启启动数据库的线程
	*/

	bool Start(CDeliverMgr *pDeliverMgr,const string& szODBC);

    void Start(const char *infoIdxPath, const char *contentPath, CDeliverMgr *pDeliverMgr);

    void Stop();

private:
    static unsigned __stdcall MonitorFunc(void *param);
	
   /**
	* 监控数据库数据函数
	* @param param 传递的线程的指针
	* @return 返回线程运行结果
	*/
	static unsigned __stdcall  MonitorDataBaseFunc(void *param);

   /**
	* 连接ODBC数据源
	*/
	void ConnectODBCDatabase();

   /**
	* VariantToCString函数将CDBVariant对象转成CString,用以显示
	* @param var 数据库种类对象指针
	* @return 返回一个CString对象
	*/
	static CString VariantToCString(CDBVariant *var);

   /**
	* 分发数据
	* @param lastMaxMid 临时的资讯主键值
	* @param curMaxMid 最大的主键值
	*/
	void DistributeInfo(int &maxMid, int &curMaxMid);

	int DistributeInfo(CRecordset& recordSet);

    void DistributeInfo(const char *filename);

    void ClearDir();

    string m_infoIndexPath;

    string m_contentPath;

    volatile bool m_end;

    CDeliverMgr *m_pDevliverMgr;

	CRecordset m_recordSet;

    char m_buf[8192];

public:
	string m_szODBC;
};
