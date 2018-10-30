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


// ��С�Ĺ̶���������
#define FIXEDPARAMS 6

const MarketInfo MARKETINFOS[] = 
{ 
    {"SH", "���ڹ�Ʊ-��֤֤ȯ"},
    {"SZ", "���ڹ�Ʊ-��֤֤ȯ"},
    {"HJ", "�ƽ�"},
    {"QH", "�ڻ�"},
    {"WH", "���"},
    {"WP", "����"},
    {"GP", "��Ʊ"},         // ��Ѷʹ��
    {"AM", "�����г�"}      // ��Ѷʹ��
};

// �����ܹ�֧��K���������ͳ���
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
    {"WEEK",     "week",     "wek"}, // mod by Jerry Lee, 2011-1-4, �����ļ���չ��
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

    setlocale(LC_ALL,"Chinese-simplified"); // �������Ļ�����Ϊ��֧��fstream��·���к������ĵ��ļ�
}

CSendCommand::~CSendCommand()
{
}

/* 
���ܣ����������ַ������ַ�����ʽΪyyyymmddhhnn
������
str: Դ�ַ���
year�������õ������
month�������õ����·�
day�������õ�������
hour: Сʱ
minute: ����
����ֵ�������ɹ�������true��ʧ���򷵻�false
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
                // ������ʷ����
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

    // added by Jerry Lee, 2011-2-28, ������Ѷ����
    if ("INFO" == hisDataInfo.dataType)
    {
        SendInfoData(hisDataInfo);

        return;
    }

    // ������ʷ����
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

// ��recv��globalfun.cpp�п��������ĺ���
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

void CSendCommand::SearchPointFromFile(_IN const char *filename, _OUT vector<StockDayT> &stVector)
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

        // �������2012��Խ�������, Jerry Lee, 2012-1-6
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
    // ����ʷ���ݿ�������������
    string strHisData;
    strHisData.append((char *)&hisData.header, sizeof(HistoryDataHeader));
    strHisData.append(hisData.content, hisData.header.length);

    // ������ʷ���ݷ��ͱ���
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
        // ������ʷ����
        HistoryData hisData;
        memset(&hisData, 0, sizeof(hisData));
        memcpy(hisData.header.marketType, m_marketInfo.marketType.c_str(), 
            m_marketInfo.marketType.length());
        memcpy(hisData.header.productCode, filecode.code.c_str(), 
            min(sizeof(hisData.header.productCode), filecode.code.length()));
        memcpy(hisData.header.dataType, hisDataInfo.dataType.c_str(), 
            hisDataInfo.dataType.length());

        vector<StockDayT> sdVector;

        // ��������������ʱ�䷶Χ������
        // �˴����Ż�Ϊ��ȡ�ļ�ӳ�䷶Χ��ֱ�ӽ��÷�Χ���ݿ�����������
        SearchPointFromFile(filecode.filename.c_str(), sdVector);

        const int COUNT = 100;

        unsigned int n = 0;

        char buf[4096];
        for (vector<StockDayT>::iterator it = sdVector.begin(); it != sdVector.end(); )
        {
            unsigned int size = min(sdVector.size() - n*COUNT, COUNT);

            hisData.header.length = size*sizeof(StockDayT);

            // ����Ƶ���ط�����ͷ��ڴ�
            //hisData.content = new char[hisData.header.length];
            hisData.content = buf;

            StockDayT &sd = *it;

            memcpy(hisData.content, (char *)(&sd), hisData.header.length);

            SendHistoryPackage(hisData);

            n++;

            //delete [] hisData.content;

            it += size;

            // ����100���룬�ó�CPUʱ��������̴߳���
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
		cout << "No hisdata sended! " << "\n";
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
        // ������ʷ����
        TickData tickData;
        memset(&tickData, 0, sizeof(tickData));
        memcpy(tickData.header.marketType, m_marketInfo.marketType.c_str(), 
            m_marketInfo.marketType.length());
        memcpy(tickData.header.productCode, filecode.code.c_str(), 
            min(sizeof(tickData.header.productCode), filecode.code.length()));
        memcpy(tickData.header.dataType, hisDataInfo.dataType.c_str(), 
            hisDataInfo.dataType.length());

        tickData.fileSize = get_filesize(filecode.filename.c_str());

        // ���ļ�
        CYLFileMapping file(filecode.filename.c_str());

        if (!file.IsOpen())
        {
			cout << "Open file " << filecode.filename.c_str() << "failed!\n";

            return;
        }

        // ����ѹ����Ĵ�С
        unsigned long nLen = (double)tickData.fileSize*1.001 + 12 + sizeof(DWORD);

        // �洢ѹ�����������buffer
        char *pZipBuf = new char[nLen];

        // ѹ���ļ�
        if (compress((BYTE*)pZipBuf, &nLen, (BYTE*)(file.GetMemory()), tickData.fileSize) == Z_OK)
        {   
            // ѹ���ɹ�
            tickData.zipSize = nLen;

            const int bufSize = 2048; // ÿ������������ݳ���

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

                // ����100���룬�ó�CPUʱ��������̴߳���
                Sleep(100);
            }
        }

        // �ͷ�ѹ�����ݻ���
        delete [] pZipBuf;

        // �ر��ļ�
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
    // ����ʷ���ݿ�������������
    string strTickData;
    strTickData.append((char *)&tickData.header, sizeof(HistoryDataHeader));
    strTickData.append((char *)&tickData.fileSize, sizeof(tickData.fileSize));
    strTickData.append((char *)&tickData.seqNo, sizeof(tickData.seqNo));
    strTickData.append((char *)&tickData.zipSize, sizeof(tickData.zipSize));
    strTickData.append(tickData.content, tickData.header.length);

    // ������ʷ���ݷ��ͱ���
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

    // ���ļ�
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
        // �����ļ���һ����¼
        InfoFile *pIF = iif.GetItem(i);

        // �г������Ƿ��������
        string strTmp;
        strTmp.append(pIF->marketType, sizeof(pIF->marketType));
        if (m_marketInfo.marketType.compare(strTmp.c_str()) != 0)
        {
            continue;
        }

        // Ʒ�������Ƿ��������
        strTmp.clear();
        strTmp.append(pIF->productCode, sizeof(pIF->productCode));
        if (m_code != strTmp && strTmp != INVALID_PRODUCTCODE && m_code != "*")
        {
            continue;
        }

        // �����Ƿ��������
        if (pIF->dateTime >= tmB && pIF->dateTime <= tmE)
        {
            sender.Send(pIF);        
        }
    }

    // �ر��ļ�
    iif.Close();
}