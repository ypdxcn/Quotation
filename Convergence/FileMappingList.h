/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	FileMappingList.h 
* Programer(s):	Jerry Lee 
* Created:      20101227 
* Description:	interface for file mapping list
*               该模板适用于读取固定结构填充的文件, 支持随机访问和顺序访问，顺序访问的效率更高
* History:
*******************************************************************************/

#ifndef _FILEMAPPINGLIST_H
#define _FILEMAPPINGLIST_H 

#pragma once

#include <string>
#include <vector>
#include "windows.h"
#include "YLFileMapping.h"

using namespace std;


template<typename T, int _SIZE=sizeof(T)>
class CFileMappingList
{
public:
    CFileMappingList() : m_strFileName("")
    {

    }

    ~CFileMappingList()
    {

    }

    // methods
public:
    // 打开文件
    void Open(const char *filename)
    {
        m_fileMapping.Open(filename);

        if (m_fileMapping.IsOpen())
        {
            m_strFileName = filename;
        }
    }

    // 关闭文件
    void Close()
    {
        m_fileMapping.Close();

        m_strFileName.clear();
    }

    // 判断文件是否已经打开
    bool IsOpen()
    {
        return !m_strFileName.empty() && m_fileMapping.IsOpen();
    }

    // 是否已经处于开端
    bool IsBof()
    {
        return m_fileMapping.IsBof();
    }

    // 是否已经处于末端
    bool IsEof()
    {
        return m_fileMapping.IsEof();
    }

    // 得到当前Item指针，并将文件指针向后移动一个Item，该方法用于顺序访问
    T *Next()
    {
        char *p = m_fileMapping.GetCurrent();
        
        m_fileMapping.Forward(_SIZE);  

        return (T *)p;
    }

    // 得到当前Item指针，并将文件指针向前移动一个Item，该方法用于顺序访问
    T *Prior()
    {
        byte *p = m_fileMapping.GetCurrent();

        m_fileMapping.Backward(_SIZE);  

        return (T *)p;
    }


    // properties
public:        
    // 根据索引得到内存地址
    T *GetItem(int index)
    {
        byte *p = m_fileMapping.GetMemory() + index*_SIZE;

        return (T *)p;
    }
        
    // 得到Item的数量
    int GetCount()
    {
        return m_fileMapping.GetSize()/_SIZE;
    }

private:
    string  m_strFileName;

    CYLFileMapping m_fileMapping;
};

#endif
