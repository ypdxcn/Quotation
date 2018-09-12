/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	InfoPublisher.cpp 
* Programer(s):	Jerry Lee 
* Created:      20110214 
* Description:	implementation of publishing information
* History:
*******************************************************************************/

#define  _WIN32_WINNT 0x0500


#include "InfoPublisher.h"
#include "windows.h"
#include "process.h"
#include "XmlInfoIndexFile.h"
#include "SamplerMsgDef.h"
#include "SamplerPacket.h"
#include <iostream>
#include "InfoSender.h"
#include <WinSock.h>

CDatabase g_DB;

inline void Multi2Wide( const std::string& szStr, std::wstring& wszStr )
{
	int nLength = MultiByteToWideChar( CP_ACP, 0, szStr.c_str(), -1, NULL, NULL );
	wszStr.resize(nLength);
	LPWSTR lpwszStr = new wchar_t[nLength + 1];
	MultiByteToWideChar( CP_ACP, 0, szStr.c_str(), -1, lpwszStr, nLength );
	wszStr = lpwszStr;
	delete [] lpwszStr;
}

inline void Wide2Multi(const std::wstring& wszStr, std::string& szStr)
{
	int nLen = WideCharToMultiByte( CP_ACP, 0, wszStr.c_str(), -1, NULL, 0, NULL, NULL );
	szStr.resize(nLen);
	char* lpStr = new char[nLen + 1];
	WideCharToMultiByte( CP_ACP, 0, wszStr.c_str(), -1, lpStr, nLen, NULL, NULL );
	szStr = lpStr;
	delete []lpStr;
}

inline void UTF82Gb2312(const char* utf8, std::string& szStr)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	if (!wstr)
		return ;
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	std::wstring wszStr = wstr;
	delete[] wstr;

	Wide2Multi(wszStr, szStr);
}

char* U2G(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);

	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if(wstr) delete[] wstr;
	return str;
}
CInfoPublisher::CInfoPublisher()
: m_end(false)
{
}

CInfoPublisher::~CInfoPublisher()
{
}

void CInfoPublisher::Start(const char *infoIdxPath, const char *contentPath, 
                           CDeliverMgr *pDeliverMgr)
{
    Stop();

    m_pDevliverMgr = pDeliverMgr;

    m_infoIndexPath = infoIdxPath;
    if (m_infoIndexPath.empty())
    {
        return;
    }

	
    m_contentPath = contentPath;
    if (m_contentPath.empty())
    {
        return;
    }

    m_end = false;

    unsigned int uThrID;
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &MonitorFunc,  this, 0, &uThrID);
}

bool CInfoPublisher::Start(CDeliverMgr *pDeliverMgr,const string& szODBC)
{
	Stop();
	m_end = false;


	m_szODBC =  szODBC;

	m_pDevliverMgr = pDeliverMgr;
	unsigned int uThreadid;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &MonitorDataBaseFunc,this, 0, &uThreadid);
	if(NULL == hThread)
	{
		return false;
	}
	return true;
}

void CInfoPublisher::Stop()
{
    m_end = true;
    Sleep(50);
}

void CInfoPublisher::ConnectODBCDatabase()
{
	
}



unsigned __stdcall  CInfoPublisher::MonitorDataBaseFunc( void *param )
{
	Sleep(20000);
	CInfoPublisher *pInfoPub = (CInfoPublisher*)param;

	int iMaxVal = 0;
	FILE * fp = fopen("InfoMaxID", "rb");
	if (fp)
	{
		fread((char*)&iMaxVal, sizeof(int), 1, fp);
		fclose(fp);
	}
	CRecordset rs(&g_DB);

	bool bQuery = FALSE;
	
	while (!bQuery)
	{
		if (!g_DB.IsOpen())
		{
			if (bQuery)
			{
				break;
			}
			BOOL bRet  = FALSE;
			try
			{
				char acDB[256];
				sprintf(acDB, "DSN=%s;", pInfoPub->m_szODBC.c_str());
				bRet = g_DB.OpenEx(acDB);

				g_DB.ExecuteSQL("set names utf8");

				
			}
			catch (CDBException* e)
			{
				TCHAR szCause[255];
				e->GetErrorMessage(szCause, sizeof(szCause));
				CRLog(E_ERROR, "%s", "Open ODBC ERROR!");
				continue;
			}
			if (!bRet)
			{
				Sleep(5000);
				CRLog(E_ERROR, "%s", "Open ODBC failed!");
				continue;
			}
		}

		try
		{
			CString strSQL;
			strSQL.Format(_T("select A.*,B.typecode from TB_MESSAGE as A join TB_MESSAGETYPE as B on  A.mtid = B.mtid where A.mid > %d"), iMaxVal);
			if (rs.Open(CRecordset::forwardOnly, (LPCTSTR)strSQL))
			{
				while(!rs.IsEOF() && !bQuery)
				{
					iMaxVal = pInfoPub->DistributeInfo(rs);
					rs.MoveNext();
					FILE * fp = fopen("InfoMaxID", "w+b");
					if (fp)
					{
						fwrite((char*)&iMaxVal, sizeof(int), 1, fp);
						fclose(fp);
					}
				}
				rs.Close();
			}
			
			Sleep(20000);
		}
		catch (CDBException* e)
		{
			TCHAR szCause[255];
			e->GetErrorMessage(szCause, sizeof(szCause));
			CRLog(E_ERROR, "数据库操作出错：%s", e->m_strError);
			continue;
		}		
	}
	g_DB.Close();

	return 0;
}

unsigned CInfoPublisher::MonitorFunc(void *param)
{
    CInfoPublisher *pPubr = (CInfoPublisher *)param;

    // 清空目录
    //pPubr->ClearDir();

    // 开始监测目录
    HANDLE hDir = CreateFile(pPubr->m_infoIndexPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hDir == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    CRLog(E_APPINFO, "Monitoring:%s",pPubr->m_infoIndexPath.c_str()); 

    while (!pPubr->m_end)
    {
        char buf[2*(sizeof(FILE_NOTIFY_INFORMATION)+MAX_PATH)];
        FILE_NOTIFY_INFORMATION *pNotify = (FILE_NOTIFY_INFORMATION *)buf;
        DWORD dwRet;
        if (ReadDirectoryChangesW(hDir, buf, sizeof(buf), TRUE, 
            FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME
            |FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE
            |FILE_NOTIFY_CHANGE_CREATION, &dwRet, NULL, NULL))
        {
            char tmp[MAX_PATH], str1[MAX_PATH];

            while (pNotify != NULL)
            {
                memset(tmp, 0, sizeof(tmp));
                WideCharToMultiByte(CP_ACP, 0, pNotify->FileName, pNotify->FileNameLength/2,
                    tmp, 99, NULL, NULL);
                strcpy(str1, tmp);

                if (FILE_ACTION_ADDED == pNotify->Action)
                {
                    string strFile = pPubr->m_infoIndexPath + "\\" + str1;

                    CRLog(E_APPINFO, "Sending:%s begin", strFile.c_str()); 

                    Sleep(50);

                    pPubr->DistributeInfo(strFile.c_str());

                    CRLog(E_APPINFO, "Sending:%s end",strFile.c_str()); 
                }

                //
                if (pNotify->NextEntryOffset != 0)
                {
                    pNotify = (PFILE_NOTIFY_INFORMATION)((char*)pNotify+pNotify->NextEntryOffset);
                }
                else
                {
                    break;
                }
            }
        }

        Sleep(50);
    }
   
    CloseHandle(hDir);

    _endthreadex(0);

    return 0;
}

int CInfoPublisher::DistributeInfo(CRecordset& recordSet)
{
	CString strSQL;
	CDBVariant tmpVar;

	// 一条资讯
	InfoData info;
	memset(&info, 0, sizeof(info));

	// 填充资讯头
	recordSet.GetFieldValue(_T("mid"), tmpVar);
	CString cStrmtid = CInfoPublisher::VariantToCString(&tmpVar);
	int mid = ::atoi(cStrmtid.GetBuffer());
	tmpVar.Clear();

	// 填充资讯头
	recordSet.GetFieldValue(_T("typecode"), tmpVar);
	CString typecode = CInfoPublisher::VariantToCString(&tmpVar);
	memcpy(info.header.marketType, typecode.GetBuffer(), typecode.GetLength());
	tmpVar.Clear();

	// 填充资讯头
	recordSet.GetFieldValue(_T("productcode"), tmpVar);
	CString strProductCode = CInfoPublisher::VariantToCString(&tmpVar);
	memcpy(info.header.productCode, strProductCode.GetBuffer(), strProductCode.GetLength());
	tmpVar.Clear();

	memcpy(info.header.dataType, "INFO", 4 );
	recordSet.GetFieldValue(_T("mtitle"), tmpVar);
	CString title = CInfoPublisher::VariantToCString(&tmpVar);
	memcpy(info.title, title.GetBuffer(), title.GetLength());
	tmpVar.Clear();

	recordSet.GetFieldValue(_T("pdate"), tmpVar);
	CString cDate = CInfoPublisher::VariantToCString(&tmpVar);
	std::string strDate = cDate.Mid(0, 10);
	std::string strTime = cDate.Mid(11, 8);

	CGessDate gDate;
	CGessTime gTime;
	gDate.FromString(strDate.c_str());
	gTime.FromString(strTime.c_str());

	struct tm tTM;
	time_t tTime;
	tTM.tm_year = gDate.Year() - 1900;
	tTM.tm_mon  = gDate.Month() - 1;
	tTM.tm_mday = gDate.Day();
	tTM.tm_hour = gTime.Hour();
	tTM.tm_min  = gTime.Minute();
	tTM.tm_sec  = gTime.Second();
	tTM.tm_isdst = -1;

	tTime = mktime(&tTM);


	info.dateTime = tTime;		//C语言时间，即从1970年1月1日零时开始的秒数
	tmpVar.Clear();

	recordSet.GetFieldValue(_T("mcontext"), tmpVar);

	CString mcontext = CInfoPublisher::VariantToCString(&tmpVar);

	info.header.length = mcontext.GetLength();	
	info.content = new char[info.header.length + 1];
	memset(info.content, 0, info.header.length + 1);
	memcpy(info.content, mcontext.GetBuffer(), info.header.length);
	tmpVar.Clear();

	//CRLog(E_APPINFO, "Content length:%d", info.header.length); 

	CInfoSender sender(m_pDevliverMgr);
	sender.SendPacket(info);

	CRLog(E_APPINFO, "Forward info:%s ", info.title); 

	if (info.content != NULL)
	{
		delete [] info.content;
		info.content = NULL;
	}

	return mid;

}
void CInfoPublisher::DistributeInfo(int &maxMid, int &curMaxMid)
{
	CString strSQL;
	CDBVariant tmpVar;
	CRecordset recordSet(&g_DB);
	strSQL.Format(_T("select A.*,B.typecode from TB_MESSAGE as A join TB_MESSAGETYPE as B on  A.mtid = B.mtid where A.mid > %d order by A.mid"), curMaxMid);
	BOOL bResult = recordSet.Open(CRecordset::forwardOnly, (LPCTSTR)strSQL);
	if (!bResult)
	{
		return;
	}

	while(!recordSet.IsEOF())
	{
		// 一条资讯
		InfoData info;
		memset(&info, 0, sizeof(info));

		// 填充资讯头
		recordSet.GetFieldValue(_T("mtid"), tmpVar);
		CString cStrmtid = CInfoPublisher::VariantToCString(&tmpVar);
		int mtid = ::atoi(cStrmtid.GetBuffer());
		tmpVar.Clear();

		// 填充资讯头
		recordSet.GetFieldValue(_T("typecode"), tmpVar);
		CString typecode = CInfoPublisher::VariantToCString(&tmpVar);
		memcpy(info.header.marketType, typecode.GetBuffer(), typecode.GetLength());
		tmpVar.Clear();

		// 填充资讯头
		recordSet.GetFieldValue(_T("productcode"), tmpVar);
		CString strProductCode = CInfoPublisher::VariantToCString(&tmpVar);
		memcpy(info.header.productCode, strProductCode.GetBuffer(), strProductCode.GetLength());
		tmpVar.Clear();

		memcpy(info.header.dataType, "INFO", 4 );
		recordSet.GetFieldValue(_T("mtitle"), tmpVar);
		CString title = CInfoPublisher::VariantToCString(&tmpVar);
		memcpy(info.title, title.GetBuffer(), title.GetLength());
		tmpVar.Clear();
		
		recordSet.GetFieldValue(_T("pdate"), tmpVar);
		CString cDate = CInfoPublisher::VariantToCString(&tmpVar);
		std::string strDate = cDate.Mid(0, 10);
		std::string strTime = cDate.Mid(11, 8);

		CGessDate gDate;
		CGessTime gTime;
		gDate.FromString(strDate.c_str());
		gTime.FromString(strTime.c_str());

		struct tm tTM;
		time_t tTime;
		tTM.tm_year = gDate.Year() - 1900;
		tTM.tm_mon  = gDate.Month() - 1;
		tTM.tm_mday = gDate.Day();
		tTM.tm_hour = gTime.Hour();
		tTM.tm_min  = gTime.Minute();
		tTM.tm_sec  = gTime.Second();
		tTM.tm_isdst = -1;

		tTime = mktime(&tTM);


		info.dateTime = tTime;		//C语言时间，即从1970年1月1日零时开始的秒数
		tmpVar.Clear();
		
		recordSet.GetFieldValue(_T("mcontext"), tmpVar);

		CString mcontext = CInfoPublisher::VariantToCString(&tmpVar);
	
		info.header.length = mcontext.GetLength();	
		info.content = new char[info.header.length + 1];
		memset(info.content, 0, info.header.length + 1);
		memcpy(info.content, mcontext.GetBuffer(), info.header.length);
		tmpVar.Clear();
	
		CInfoSender sender(m_pDevliverMgr);
		sender.SendPacket(info);

		CRLog(E_APPINFO, "Forward info:%s ", info.title); 

		if (info.content != NULL)
		{
			delete [] info.content;
			info.content = NULL;
		}
		
		recordSet.MoveNext();
	}
    recordSet.Close();
}

void CInfoPublisher::DistributeInfo(const char *filename)
{
    CXmlInfoIndexFile infoIdx;

    // 打开文件
    if (!infoIdx.Open(filename))
    {
        //DeleteFile(filename);

        // added by Jerry Lee, 2012-2-23

        CRLog(E_APPINFO, "Could not open info file: %s", filename); 

        return;
    }

    try
    {
        CInfoSender sender(m_contentPath.c_str(), m_pDevliverMgr);

        int n = 0;
        for (int i = 0; i < infoIdx.GetCount(); i++)
        {
            // 索引文件的一条记录
            InfoFile *pIF = infoIdx.GetItem(i);

            // 发送
            sender.Send(pIF);

            // 休眠100毫秒，让出CPU时间给其它线程处理
            Sleep(100);
        }
    }
    catch(std::exception e)
    {
        CRLog(E_ERROR,"exception:%s!",e.what());
    }
    catch(...)
    {
        CRLog(E_ERROR,"%s","Unknown exception!");
    }

    // 关闭文件
    infoIdx.Close();

    // 删除文件
    //DeleteFile(filename);
}

void CInfoPublisher::ClearDir()
{
    WIN32_FIND_DATA wfd;   
    string str = m_infoIndexPath + "\\*.*";

    HANDLE hFind = FindFirstFile(str.c_str(), &wfd);   

    if (INVALID_HANDLE_VALUE == hFind)   
    {   
        return;
    }

    do  
    {   
        if(wfd.cFileName[0] != '.')   //   .   ..   
        {   
            str = m_infoIndexPath + "\\" + wfd.cFileName;  
            DeleteFile(str.c_str());
        }   
       
    } while(FindNextFile(hFind, &wfd));  

    FindClose(hFind);   
}

CString CInfoPublisher::VariantToCString( CDBVariant *var )
{
	CString str;//转换以后的字符串
	if(!var)
	{
		str = "NULL Var Parameter";
		return str;
	}

	switch(var->m_dwType)
	{
	case DBVT_SHORT:
		str.Format(_T("%d"), (int)var->m_iVal);
		break;
	case DBVT_LONG:
		str.Format(_T("%ld"), var->m_lVal);
		break;
	case DBVT_SINGLE:
		str.Format(_T("%10.6f"), (double)var->m_fltVal);
		break;
	case DBVT_DOUBLE:
		str.Format(_T("%.6f"), (double)var->m_fltVal);
		break;
	case DBVT_BINARY:
		str.Format(_T("%s"), var->m_pbinary);
		break;
	case DBVT_NULL:
		str = "(NULL)";
		break;
	case DBVT_BOOL:
		str = (var->m_boolVal==0)?_T("FALSE"):_T("TRUE");
		break;
	case DBVT_STRING:
		str = var->m_pstring->GetBuffer();
		break;
	case DBVT_ASTRING:
		{
			str = var->m_pstringA->GetBuffer();
			string szStr;
			UTF82Gb2312(str,szStr);
			str = szStr.c_str();
		}
		break;
	case DBVT_WSTRING:
		{
			CStringW wstr;
			wstr = var->m_pstringW->GetBuffer(var->m_pstringW->GetLength());
			wstring wszStr = wstr.GetBuffer(wstr.GetLength());
			string szStr;
			Wide2Multi(wszStr, szStr);
			str = szStr.c_str();
		}
		break;
	case DBVT_DATE:
		str.Format(_T("%d-%d-%d"), (var->m_pdate)->year, (var->m_pdate)->month, (var->m_pdate)->day);
		break;
	default:
		str.Format(_T("Unknow type %d\n"), var->m_dwType);
	}
	return str;
}

