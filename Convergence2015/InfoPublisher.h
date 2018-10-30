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
#include <thread>

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
	* �����������ݿ���߳�
	*/

	bool Start(CDeliverMgr *pDeliverMgr,const string& szODBC);

    void Start(const char *infoIdxPath, const char *contentPath, CDeliverMgr *pDeliverMgr);

    void Stop();

private:
    static unsigned __stdcall MonitorFunc(void *param);
	
   /**
	* ������ݿ����ݺ���
	* @param param ���ݵ��̵߳�ָ��
	* @return �����߳����н��
	*/
	static unsigned __stdcall  MonitorDataBaseFunc(void *param);

   /**
	* ����ODBC����Դ
	*/
	void ConnectODBCDatabase();

   /**
	* VariantToCString������CDBVariant����ת��CString,������ʾ
	* @param var ���ݿ��������ָ��
	* @return ����һ��CString����
	*/
	static CString VariantToCString(CDBVariant *var);

   /**
	* �ַ�����
	* @param lastMaxMid ��ʱ����Ѷ����ֵ
	* @param curMaxMid ��������ֵ
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


	std::thread  m_ScanDBThread;
	std::thread  m_ScanFileThread;
public:
	string m_szODBC;
};
