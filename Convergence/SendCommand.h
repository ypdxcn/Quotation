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


using namespace std;

// 日期时间结构定义
typedef struct tagYLDateTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} YLDateTime;


// 定义市场类型结构
typedef struct tagMarketInfo
{
    string marketType;       // 市场类型缩写
    string dirPrefix;        // 市场数据所在目录前缀
} MarketInfo;


// 定义历史数据信息类型结构
typedef struct tagHisDataInfo
{
    string dataType;         // 历史数据类型缩写
    string dir;              // 历史数据目录名
    string fileExt;          // 数据文件扩展名
} HisDataInfo;


// 定义文件名和品种代码的配对
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
    功能：执行发送数据命令

    参数：
    params：命令参数列表
    
    返回值：执行发送命令成功返回true，否则返回false
    */
    bool Execute(const vector<string> &params);

private:
    bool GetMarketInfo(const string &str);

    bool ParseDate(const string &str, int &year, int &month, int &day,
        int &hour, int &minute);

    int GetDataType(const string &str);

    // 根据用户命令信息查找文件
    void FindDataFile(const HisDataInfo &hisDataInfo, vector<FileCode> &files);

    // 发送所有历史数据文件
    void SendHistoryData(const HisDataInfo &hisDataInfo);

    // 发送单个历史数据文件
    void SendHistoryFile(const HisDataInfo &hisDataInfo, const FileCode &filecode);

    // 发送一个历史数据包
    void SendHistoryPackage(HistoryData &hisData); 
 
    // 从数据文件中查找符合条件的数据记录
    void SearchPointFromFile(const char *filename, vector<StockDayT> &stVector);

    // 发送tick数据
    void SendTickData(const HisDataInfo &hisDataInfo);

    // 发送单个tick数据文件
    void SendTickFile(const HisDataInfo &hisDataInfo, const FileCode &filecode);

    // 发送tick数据包
    void SendTickPackage(TickData &tickData);

    // 查找tick文件
    void FindTickFile(const HisDataInfo &hisDataInfo, vector<FileCode> &files);

    // 发送资讯文件
    void SendInfoData(const HisDataInfo &hisDataInfo);

    void SendInfoIndex(const char *filename);

    MarketInfo m_marketInfo;
    string m_code;
    string m_nodeId;
    YLDateTime m_dtBegin, m_dtEnd;
    vector<HisDataInfo> m_hisDataInfos;
    string m_strHisDataPath;
    string m_strTickDataPath;

    // added by Jerry Lee, 2011-2-28, 资讯保存路径
    string m_strInfoIndexPath;
    string m_strContentPath;

    CDeliverMgr *m_pDeliverMgr;
};

#endif


