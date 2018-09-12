/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      cvg 
* File Name:	XmlInfoIndexFile.h 
* Programer(s):	Jerry Lee 
* Created:      20110224 
* Description:	interface for xml information index file
* History:
*******************************************************************************/


#pragma once


#include <vector>

using namespace std;

#ifndef MAX_LENGTH
#define MAX_LENGTH 256
#endif

// 资讯文件格式
// 资讯索引文件的一条记录
typedef struct tagInfoFile
{
    char marketType[12];      //kenny 扩充类型  2014-1-9
    char productCode[6];
    char title[128];
    int dateTime;
    char path[MAX_LENGTH];
    int fileSize;
} InfoFile;


// 资讯索引文件
typedef struct tagInfoIndex
{
    int version;
    int count;
    InfoFile infoFiles[1];
} InfoIndex;

//
class CXmlInfoIndexFile
{
public:
    CXmlInfoIndexFile();
    ~CXmlInfoIndexFile();
public:
    // 打开文件
    bool Open(const char *filename);

    // 关闭文件
    void Close();

    // 根据索引得到内存地址
    InfoFile *GetItem(int index);

    // 得到Item的数量
    int GetCount()
    {
        return static_cast<int>(m_infofiles.size());
    }

private:
    InfoIndex *m_pInfoIndex;

    vector<InfoFile> m_infofiles;

    double m_version;

    int m_count;
};
