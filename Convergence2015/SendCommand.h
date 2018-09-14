/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	SendCommand.h 
* Programer(s):	Jerry Lee 
* Created:      20101217 
* Description:	interface for "send" command
* History:
*******************************************************************************/

#ifndef _SEND_COMMAND_
#define _SEND_COMMAND_ 

#pragma once

#include <vector>
#include <string>
#include "DeliverMgr.h"
#include "XmlInfoIndexFile.h"

#define _IN
#define _OUT

using namespace std;

// ����ʱ��ṹ����
typedef struct tagYLDateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} YLDateTime;


// �����г����ͽṹ
typedef struct tagMarketInfo
{
    string marketType;       // �г�������д
    string dirPrefix;        // �г���������Ŀ¼ǰ׺
} MarketInfo;


// ������ʷ������Ϣ���ͽṹ
typedef struct tagHisDataInfo
{
    string dataType;         // ��ʷ����������д
    string dir;              // ��ʷ����Ŀ¼��
    string fileExt;          // �����ļ���չ��
} HisDataInfo;


// �����ļ�����Ʒ�ִ�������
typedef struct tagFileCode
{
    string filename;
    string code;
} FileCode;

//
class CSendCommand
{
public:
    CSendCommand(CDeliverMgr *pDeliverMgr, const string &hisDataPath,
        const string &tickDataPath, const string &infoIndexPath, 
        const string &contentPath);
    ~CSendCommand();

public:
    /*
    ���ܣ�ִ�з�����������

    ������
    params����������б�
    
    ����ֵ��ִ�з�������ɹ�����true�����򷵻�false
    */
    bool Execute(const vector<string> &params);

private:
    bool GetMarketInfo(const string &str);

    bool ParseDate(const string &str, int &year, int &month, int &day,
        int &hour, int &minute);

    int GetDataType(const string &str);

    // �����û�������Ϣ�����ļ�
    void FindDataFile(const HisDataInfo &hisDataInfo, vector<FileCode> &files);

    // ����������ʷ�����ļ�
    void SendHistoryData(const HisDataInfo &hisDataInfo);

    // ���͵�����ʷ�����ļ�
    void SendHistoryFile(const HisDataInfo &hisDataInfo, const FileCode &filecode);

    // ����һ����ʷ���ݰ�
    void SendHistoryPackage(HistoryData &hisData); 
 
    // �������ļ��в��ҷ������������ݼ�¼
    void SearchPointFromFile(const char *filename, vector<StockDayT> &stVector);

    // ����tick����
    void SendTickData(const HisDataInfo &hisDataInfo);

    // ���͵���tick�����ļ�
    void SendTickFile(const HisDataInfo &hisDataInfo, const FileCode &filecode);

    // ����tick���ݰ�
    void SendTickPackage(TickData &tickData);

    // ����tick�ļ�
    void FindTickFile(const HisDataInfo &hisDataInfo, vector<FileCode> &files);

    // ������Ѷ�ļ�
    void SendInfoData(const HisDataInfo &hisDataInfo);

    void SendInfoIndex(const char *filename);

    MarketInfo m_marketInfo;
    string m_code;
    string m_nodeId;
    YLDateTime m_dtBegin, m_dtEnd;
    vector<HisDataInfo> m_hisDataInfos;
    string m_strHisDataPath;
    string m_strTickDataPath;

    // added by Jerry Lee, 2011-2-28, ��Ѷ����·��
    string m_strInfoIndexPath;
    string m_strContentPath;

    CDeliverMgr *m_pDeliverMgr;
};

#endif


