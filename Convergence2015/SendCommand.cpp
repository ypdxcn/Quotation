/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	SendCommand.cpp 
* Programer(s):	Jerry Lee 
* Created:      20101217 
* Description:	implementation of "send" command
* History:
*******************************************************************************/

#include "SendCommand.h"
#include <iostream>
#include "SamplerPacket.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include "FileMappingList.h"
#include "YLFileMapping.h"
#include "zlib.h"
#include "InfoSender.h"


// 最小的固定参数数量
#define FIXEDPARAMS 6

const MarketInfo MARKETINFOS[] = 
{ 
    {"SH", "国内股票-上证证券"},
    {"SZ", "国内股票-深证证券"},
    {"HJ", "黄金"},
    {"QH", "期货"},
    {"WH", "外汇"},
    {"WP", "外盘"},
    {"GP", "股票"},         // 资讯使用
    {"AM", "大众市场"}      // 资讯使用
};

// 定义能够支持K线数据类型常量
const HisDataInfo HISDATAINFOS[] =
{
    {"MIN1",     "min1",     "nmn"},
    {"MIN5",     "min",      "nmn"},
    {"MIN15",    "min15",    "15"},
    {"MIN30",    "min30",    "30"},
    {"MIN60",    "min60",    "60"},
    {"MIN120",   "min120",   "120"},
    {"MIN180",   "min180",   "180"},
    {"DAY",      "day",      "day"},
    {"WEEK",     "week",     "wek"}, // mod by Jerry Lee, 2011-1-4, 修正文件扩展名
    {"MONTH",    "month",    "mnt"},
    {"SEASON",   "season",   "season"},
    {"YEAR",     "year",     "year"},
    {"TICK",     "tick",     "tick"},
    {"TREND",    "trend",    "trend"},
    {"INFO",     "info",     "info"}
}; 


//
#define INVALID_PRODUCTCODE ("FFFFFF")

//
CSendCommand::CSendCommand(CDeliverMgr *pDeliverMgr, const string &hisDataPath,
                           const string &tickDataPath, const string &infoIndexPath, 
                           const string &contentPath) 
{
    m_pDeliverMgr = pDeliverMgr;

    m_strHisDataPath = hisDataPath;

    m_strTickDataPath = tickDataPath;

    m_strInfoIndexPath = infoIndexPath;

    m_strContentPath = contentPath;

    setlocale(LC_ALL,"Chinese-simplified"); // 设置中文环境，为了支持fstream打开路径中含有中文的文件
}

CSendCommand::~CSendCommand()
{
}

/* 
功能：解析日期字符串，字符串格式为yyyymmddhhnn
参数：
str: 源字符串
year：解析得到的年份
month：解析得到的月份
day：解析得到的日子
hour: 小时
minute: 分钟
返回值：解析成功，返回true，失败则返回false
*/
bool CSendCommand::ParseDate(const string &str, int &year, int &month, int &day,
                             int &hour, int &minute)
{
    bool ret;

    if (str.size() == 12)
    {
        year = atoi(str.substr(0, 4).c_str());   
        month = atoi(str.substr(4, 2).c_str());
        day = atoi(str.substr(6, 2).c_str());
        hour = atoi(str.substr(8, 2).c_str());   
        minute = atoi(str.substr(10, 2).c_str());

        ret = true;
    }
    else
    {
        year = 0;

        ret = false;
    }

    return ret;
}

bool CSendCommand::GetMarketInfo(const string &str)
{
    bool ret = false;

    for (int i = 0; i < sizeof(MARKETINFOS)/sizeof(MARKETINFOS[0]); i++)
    {
        string str1 = str;
        transform(str1.begin(), str1.end(), str1.begin(), toupper);

        if (str1 == MARKETINFOS[i].marketType)
        {
            m_marketInfo = MARKETINFOS[i];

            ret = true;

            break;
        }
    }

    return ret;
}

int CSendCommand::GetDataType(const string &str)
{   
    return 0;
}

bool CSendCommand::Execute(const vector<string> &params)
{
    if (params.size() < FIXEDPARAMS)
    {
        return false;
    }

    m_nodeId = params[0];

    if (!GetMarketInfo(params[1]))
    {
        cout << "Can not find market type named " << params[1] << "\n";
        return false;
    }

    m_code = params[2];

    if (!ParseDate(params[3], m_dtBegin.year, m_dtBegin.month, m_dtBegin.day, 
        m_dtBegin.hour, m_dtBegin.minute))
    {
        //return false;
    }

    if (!ParseDate(params[4], m_dtEnd.year, m_dtEnd.month, m_dtEnd.day, m_dtEnd.hour, 
        m_dtEnd.minute))
    {
        //return false;
    }

    for (unsigned int i = 5; i < params.size(); i++)
    {
        for (int j = 0; j < sizeof(HISDATAINFOS)/sizeof(HISDATAINFOS[0]); j++)
        {
            string str = params[i];
            transform(str.begin(), str.end(), str.begin(), toupper);

            if (str == HISDATAINFOS[j].dataType)
            {
                // 发送历史数据
                SendHistoryData(HISDATAINFOS[j]);

                break;
            }
        }
    }

    return true;
}


void CSendCommand::FindDataFile(const HisDataInfo &hisDataInfo, vector<FileCode> &files)
{
    string strPath = m_strHisDataPath + "\\" 
        + m_marketInfo.dirPrefix + "*";

    WIN32_FIND_DATA wfd;
    
    HANDLE hFindFile = ::FindFirstFile(strPath.c_str(), &wfd);

    if (INVALID_HANDLE_VALUE == hFindFile) 
    {
        return; 
    }

    do 
    {
        if (wfd.dwFileAttributes && FILE_ATTRIBUTE_DIRECTORY)
        {
            string strFileName = m_strHisDataPath + "\\" + wfd.cFileName + "\\" 
                + hisDataInfo.dir + "\\" + m_code + "." + hisDataInfo.fileExt;


#if 0
            if (::access(strFileName.c_str(), 0) == 0)
            {
                ret = strFileName;

                break;
            }
#endif       
            WIN32_FIND_DATA wfd1;
            HANDLE hFile = FindFirstFile(strFileName.c_str(), &wfd1);

            if (hFile != INVALID_HANDLE_VALUE) 
            {
                do 
                {
                    if (wfd1.dwFileAttributes && FILE_ATTRIBUTE_ARCHIVE)
                    {
                        string strCode = wfd1.cFileName;

                        int pos = strCode.find('.');
                        if (pos != string::npos)
                        {
                            strCode.erase(pos);
                        }
                        else
                        {
                             continue;
                        }

                        FileCode fc;
                        fc.filename = m_strHisDataPath + "\\" + wfd.cFileName + "\\" 
                            + hisDataInfo.dir + "\\" + wfd1.cFileName;
                        fc.code = strCode;
                        files.push_back(fc);
                    }
                } while (FindNextFile(hFile, &wfd1));

                ::FindClose(hFile);
            }
        }

    } while (FindNextFile(hFindFile, &wfd));

    ::FindClose(hFindFile);
}

int get_filesize(const char *filename)
{
    int ret = 0;

    FILE *file = fopen(filename, "rb");
    if (file)
    {
        ret = filelength(fileno(file));

        fclose(file);
    }

    return ret;
}


void CSendCommand::SendHistoryData(const HisDataInfo &hisDataInfo)
{
    //
    if ("TICK" == hisDataInfo.dataType || "TREND" == hisDataInfo.dataType)
    {
        SendTickData(hisDataInfo);

        return;
    }

    // added by Jerry Lee, 2011-2-28, 发送资讯数据
    if ("INFO" == hisDataInfo.dataType)
    {
        SendInfoData(hisDataInfo);

        return;
    }

    // 发送历史数据
    vector<FileCode> files;

    FindDataFile(hisDataInfo, files);

    if (files.empty())
    {
        return;
    }

    for (vector<FileCode>::iterator it = files.begin(); it != files.end(); it++)
    {
        cout << "Start sending " << (*it).code << "\n";
        SendHistoryFile(hisDataInfo, *it);
        cout << "Stop sending " << (*it).code << "\n";
    }

    files.clear();
}

// 从recv的globalfun.cpp中拷贝过来的函数
time_t ULongToTime(unsigned long lDate)
{
    struct tm atm;
    memset(&atm, 0, sizeof(tm));
    char szTime[32];
    sprintf(szTime, "%ld", lDate);
    int nLen = strlen(szTime);
    if(nLen <= 8)
    {
        atm.tm_year = int(lDate/10000);
        if(atm.tm_year < 1900)
        {
            atm.tm_mon    = atm.tm_year/100 - 1;
            atm.tm_mday   = atm.tm_year%100;
            atm.tm_hour   = (int)((lDate%10000)/100);
            atm.tm_min    = (int)(lDate%100);
            time_t ct     = time(NULL);
            atm.tm_year   = localtime(&ct)->tm_year + 1900;
        }
        else
        {
            atm.tm_mon    = (int)((lDate%10000)/100) - 1;
            atm.tm_mday   = (int)(lDate%100);
            atm.tm_hour   = 0;
            atm.tm_min    = 0;
        }
    }
    else if(nLen > 8)
    {
        atm.tm_year = (int)(lDate/100000000 + 1990);
        int nTemp   = (int)(lDate%100000000)/10000;
        atm.tm_mon  = nTemp/100 - 1;
        atm.tm_mday = nTemp%100;
        atm.tm_hour = (int)((lDate%10000)/100);
        atm.tm_min  = (int)(lDate%100);
    }
    atm.tm_year -= 1900;
    if(atm.tm_mday < 1 || atm.tm_mday > 31 || 
        atm.tm_mon  < 0 || atm.tm_mon  > 11 ||
        atm.tm_hour < 0 || atm.tm_hour > 23 ||
        atm.tm_min  < 0 || atm.tm_min  > 59 ||
        atm.tm_sec  < 0 || atm.tm_sec  > 59)
    {
        return -1;
    }
    return mktime(&atm);
}

time_t YLDateTimeToTime(const YLDateTime &dt)
{
    struct tm atm;

    memset(&atm, 0, sizeof(tm));

    atm.tm_year = dt.year - 1900;
    atm.tm_mon = dt.month - 1;
    atm.tm_mday = dt.day;
    atm.tm_hour = dt.hour;
    atm.tm_min = dt.minute;

    return mktime(&atm);
}

void CSendCommand::SearchPointFromFile(const char *filename, vector<StockDayT> &stVector)
{
    CFileMappingList<StockDayT> fml;
    
    fml.Open(filename);

    if (!fml.IsOpen())
    {
        return;
    }

    time_t tmB, tmE;
    tmB = YLDateTimeToTime(m_dtBegin);
    tmE = YLDateTimeToTime(m_dtEnd);

    while(!fml.IsEof())
    {
        StockDayT *pSD = fml.Next();

        // 解决日期2012年越界的问题, Jerry Lee, 2012-1-6
        time_t t = ULongToTime(pSD->m_lDate);

        if (t >= tmB && t <= tmE)
        {
            stVector.push_back(*pSD);
        }
    }

    fml.Close();
}

void CSendCommand::SendHistoryPackage(HistoryData &hisData)
{
    // 将历史数据拷贝到报文内容
    string strHisData;
    strHisData.append((char *)&hisData.header, sizeof(HistoryDataHeader));
    strHisData.append(hisData.content, hisData.header.length);

    // 生成历史数据发送报文
    CSamplerPacket oPkt(YL_HISTORYDATA);
    CMessageImpl &msg = dynamic_cast<CMessageImpl &>(oPkt.GetMsg());	
    msg.SetBinaryField(MSG_HISTORY_DATA, strHisData);
    unsigned int uiNodeID = atoi(m_nodeId.c_str());
    msg.SetField(MSG_NODE_ID, uiNodeID);
    unsigned int uiSeqNo = 0;
    msg.SetField(MSG_SEQ_ID, uiSeqNo);

    m_pDeliverMgr->HandleHistoryData(oPkt, uiNodeID);
}

void CSendCommand::SendHistoryFile(const HisDataInfo &hisDataInfo, 
                                   const FileCode &filecode) 
{
    try
    {
        // 生成历史数据
        HistoryData hisData;
        memset(&hisData, 0, sizeof(hisData));
        memcpy(hisData.header.marketType, m_marketInfo.marketType.c_str(), 
            m_marketInfo.marketType.length());
        memcpy(hisData.header.productCode, filecode.code.c_str(), 
            min(sizeof(hisData.header.productCode), filecode.code.length()));
        memcpy(hisData.header.dataType, hisDataInfo.dataType.c_str(), 
            hisDataInfo.dataType.length());

        vector<StockDayT> sdVector;

        // 搜索符合命令中时间范围的数据
        // 此处可优化为获取文件映射范围，直接将该范围数据拷贝到报文中
        SearchPointFromFile(filecode.filename.c_str(), sdVector);

        const int COUNT = 100;

        unsigned int n = 0;

        char buf[4096];
        for (vector<StockDayT>::iterator it = sdVector.begin(); it != sdVector.end(); )
        {
            unsigned int size = min(sdVector.size() - n*COUNT, COUNT);

            hisData.header.length = size*sizeof(StockDayT);

            // 避免频繁地分配和释放内存
            //hisData.content = new char[hisData.header.length];
            hisData.content = buf;

            StockDayT &sd = *it;

            memcpy(hisData.content, (char *)(&sd), hisData.header.length);

            SendHistoryPackage(hisData);

            n++;

            //delete [] hisData.content;

            it += size;

            // 休眠100毫秒，让出CPU时间给其它线程处理
            Sleep(100);
        }

        sdVector.clear();
    }
    catch(std::exception e)
    {
        CRLog(E_ERROR,"exception:%s!",e.what());
    }
    catch(...)
    {
        CRLog(E_ERROR,"%s","Unknown exception!");
    }
}

void CSendCommand::SendTickData(const HisDataInfo &hisDataInfo)
{
    vector<FileCode> files;

    FindTickFile(hisDataInfo, files);

    if (files.empty())
    {
        return;
    }

    for (vector<FileCode>::iterator it = files.begin(); it != files.end(); it++)
    {
        cout << "Start sending " << (*it).code << "\n";
        SendTickFile(hisDataInfo, *it);
        cout << "Stop sending " << (*it).code << "\n";
    }

    files.clear();
}

void CSendCommand::SendTickFile(const HisDataInfo &hisDataInfo, const FileCode &filecode)
{
    try
    {
        // 生成历史数据
        TickData tickData;
        memset(&tickData, 0, sizeof(tickData));
        memcpy(tickData.header.marketType, m_marketInfo.marketType.c_str(), 
            m_marketInfo.marketType.length());
        memcpy(tickData.header.productCode, filecode.code.c_str(), 
            min(sizeof(tickData.header.productCode), filecode.code.length()));
        memcpy(tickData.header.dataType, hisDataInfo.dataType.c_str(), 
            hisDataInfo.dataType.length());

        tickData.fileSize = get_filesize(filecode.filename.c_str());

        // 打开文件
        CYLFileMapping file(filecode.filename.c_str());

        if (!file.IsOpen())
        {
			cout << "Open file " << filecode.filename.c_str() << "failed!\n";

            return;
        }

        // 计算压缩后的大小
        unsigned long nLen = (double)tickData.fileSize*1.001 + 12 + sizeof(DWORD);

        // 存储压缩数据所需的buffer
        char *pZipBuf = new char[nLen];

        // 压缩文件
        if (compress((BYTE*)pZipBuf, &nLen, (BYTE*)(file.GetMemory()), tickData.fileSize) == Z_OK)
        {   
            // 压缩成功
            tickData.zipSize = nLen;

            const int bufSize = 2048; // 每个包的最大数据长度

            int count = tickData.zipSize/bufSize + ((tickData.zipSize%bufSize)?1:0);
 
            char buf[bufSize];

            char *pTmpBuf = pZipBuf;

            for (int i = 0; i < count; i++)
            {
                unsigned int size = min(tickData.zipSize - i*bufSize, bufSize);

                tickData.seqNo = i;

                tickData.header.length = size;

                tickData.content = buf;

                memcpy(tickData.content, pTmpBuf, size);

                SendTickPackage(tickData);

                pTmpBuf += size;

                // 休眠100毫秒，让出CPU时间给其它线程处理
                Sleep(100);
            }
        }

        // 释放压缩数据缓冲
        delete [] pZipBuf;

        // 关闭文件
        file.Close();
    }
    catch(std::exception e)
    {
        CRLog(E_ERROR,"exception:%s!",e.what());
    }
    catch(...)
    {
        CRLog(E_ERROR,"%s","Unknown exception!");
    }
}

void CSendCommand::FindTickFile(const HisDataInfo &hisDataInfo, vector<FileCode> &files)
{
    string strFileName = m_strTickDataPath + "\\" + m_marketInfo.dirPrefix 
        + "*-" + hisDataInfo.fileExt + "." + m_code;

    WIN32_FIND_DATA wfd;
    HANDLE hFile = FindFirstFile(strFileName.c_str(), &wfd);

    if (hFile != INVALID_HANDLE_VALUE) 
    {
        do 
        {
            if (wfd.dwFileAttributes && FILE_ATTRIBUTE_ARCHIVE)
            {
                string strCode = wfd.cFileName;

                strCode = strCode.substr(strCode.rfind('.')+1);

                if (strCode.empty())
                {
                    continue;
                }

                FileCode fc;
                fc.filename = m_strTickDataPath + "\\" + wfd.cFileName;
                fc.code = strCode;
                files.push_back(fc);
            }
        } while (FindNextFile(hFile, &wfd));

        ::FindClose(hFile);
    }
}

void CSendCommand::SendTickPackage(TickData &tickData)
{
    // 将历史数据拷贝到报文内容
    string strTickData;
    strTickData.append((char *)&tickData.header, sizeof(HistoryDataHeader));
    strTickData.append((char *)&tickData.fileSize, sizeof(tickData.fileSize));
    strTickData.append((char *)&tickData.seqNo, sizeof(tickData.seqNo));
    strTickData.append((char *)&tickData.zipSize, sizeof(tickData.zipSize));
    strTickData.append(tickData.content, tickData.header.length);

    // 生成历史数据发送报文
    CSamplerPacket oPkt(YL_TICKDATA);
    CMessageImpl &msg = dynamic_cast<CMessageImpl &>(oPkt.GetMsg());	
    msg.SetBinaryField(MSG_TICK_DATA, strTickData);
    unsigned int uiNodeID = atoi(m_nodeId.c_str());
    msg.SetField(MSG_NODE_ID, uiNodeID);
    unsigned int uiSeqNo = 0;
    msg.SetField(MSG_SEQ_ID, uiSeqNo);

    //
    m_pDeliverMgr->HandleHistoryData(oPkt, uiNodeID);
}

void CSendCommand::SendInfoData(const HisDataInfo &hisDataInfo)
{
    WIN32_FIND_DATA wfd;   
    string str = m_strInfoIndexPath + "\\*.*";

    HANDLE hFind = FindFirstFile(str.c_str(), &wfd);   

    if (INVALID_HANDLE_VALUE == hFind)   
    {   
        return;
    }

    do  
    {   
        if(wfd.cFileName[0] != '.')   //   .   ..   
        {   
            str = m_strInfoIndexPath + "\\" + wfd.cFileName; 

            SendInfoIndex(str.c_str());
        }   

    } while(FindNextFile(hFind, &wfd));  

    FindClose(hFind);   
}

void CSendCommand::SendInfoIndex(const char *filename)
{
    CXmlInfoIndexFile iif;

    // 打开文件
    if (!iif.Open(filename))
    {
        return;
    }

    time_t tmB, tmE;
    tmB = YLDateTimeToTime(m_dtBegin);
    tmE = YLDateTimeToTime(m_dtEnd);

    unsigned int uNodeId = atoi(m_nodeId.c_str());
    CInfoSender sender(m_strContentPath.c_str(), m_pDeliverMgr, uNodeId);

    for (int i = 0; i < iif.GetCount(); i++)
    {
        // 索引文件的一条记录
        InfoFile *pIF = iif.GetItem(i);

        // 市场类型是否符合条件
        string strTmp;
        strTmp.append(pIF->marketType, sizeof(pIF->marketType));
        if (m_marketInfo.marketType != strTmp)
        {
            continue;
        }

        // 品种类型是否符合条件
        strTmp.clear();
        strTmp.append(pIF->productCode, sizeof(pIF->productCode));
        if (m_code != strTmp && strTmp != INVALID_PRODUCTCODE && m_code != "*")
        {
            continue;
        }

        // 日期是否符合条件
        if (pIF->dateTime >= tmB && pIF->dateTime <= tmE)
        {
            sender.Send(pIF);        
        }
    }

    // 关闭文件
    iif.Close();
}