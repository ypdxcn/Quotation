/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      cvg 
* File Name:	XmlInfoIndexFile.cpp 
* Programer(s):	Jerry Lee 
* Created:      20110224 
* Description:	implementation of xml information index file
* History:
*******************************************************************************/

#include "XmlInfoIndexFile.h"
#include "Markup.h"
#include "Logger.h"

//
#define TAG_INFOINDEX       "InfoIndex"
#define TAG_VERSION         "Version"
#define TAG_INFOFILE        "infofile"
#define TAG_COUNT           "count"
#define TAG_MARKETTYPE      "marketType"
#define TAG_PRODUCTCODE     "productCode"
#define TAG_TITLE           "title"
#define TAG_DATETIME        "dateTime"
#define TAG_PATH            "path"
#define TAG_FILESIZE        "fileSize"

#define MIN(X, Y) ((X)<(Y)?(X):(Y))

//
CXmlInfoIndexFile::CXmlInfoIndexFile()
{
}

CXmlInfoIndexFile::~CXmlInfoIndexFile()
{
}

bool CXmlInfoIndexFile::Open(const char *filename)
{
    Close();

    CMarkup xmlDoc;

    // 载入文件
    if (!xmlDoc.Load(filename))
    {
        CRLog(E_ERROR,"could not load:%s!",filename);
        return false;
    }

    // 解析文件  

    // 文件复位
    xmlDoc.ResetMainPos();

    //
    if (xmlDoc.FindElem(TAG_INFOINDEX))
    {
        m_version = atof(xmlDoc.GetAttrib(TAG_VERSION).c_str());

        //
        xmlDoc.ResetChildPos();

        while(xmlDoc.FindChildElem(TAG_INFOFILE))
        {
            xmlDoc.IntoElem();

            InfoFile item;
            memset(&item, 0, sizeof(item));

            string strTmp;

            // get market type
            xmlDoc.ResetChildPos();
            if (xmlDoc.FindChildElem(TAG_MARKETTYPE))
            {
                strTmp = xmlDoc.GetChildData();
                memcpy(item.marketType, strTmp.c_str(), MIN(sizeof(item.marketType), strTmp.length()));
            }

            // get product code
            xmlDoc.ResetChildPos();
            if (xmlDoc.FindChildElem(TAG_PRODUCTCODE))
            {
                strTmp = xmlDoc.GetChildData();
                memcpy(item.productCode, strTmp.c_str(), MIN(sizeof(item.productCode), strTmp.length()));
            }

            // get title
            xmlDoc.ResetChildPos();
            if (xmlDoc.FindChildElem(TAG_TITLE))
            {
                strTmp = xmlDoc.GetChildData();
                memcpy(item.title, strTmp.c_str(), MIN(sizeof(item.title), strTmp.length()));
            }

            // get datetime
            xmlDoc.ResetChildPos();
            if (xmlDoc.FindChildElem(TAG_DATETIME))
            {
                item.dateTime = atoi(xmlDoc.GetChildData().c_str());
            }

            // get path
            xmlDoc.ResetChildPos();
            if (xmlDoc.FindChildElem(TAG_PATH))
            {
                strTmp = xmlDoc.GetChildData();
                memcpy(item.path, strTmp.c_str(), MIN(sizeof(item.path), strTmp.length()));
            }

            // get filesize
            xmlDoc.ResetChildPos();
            if (xmlDoc.FindChildElem(TAG_FILESIZE))
            {
                item.fileSize = atoi(xmlDoc.GetChildData().c_str());
            }

            m_infofiles.push_back(item);

            xmlDoc.OutOfElem();
        }
    }

    return m_infofiles.size() > 0;
}

void CXmlInfoIndexFile::Close()
{
    m_infofiles.clear();
}

InfoFile *CXmlInfoIndexFile::GetItem(int index)
{
    if (GetCount() > 0)
    {
        return &(m_infofiles[index]);
    }
    else
    {
        return NULL;
    }
}