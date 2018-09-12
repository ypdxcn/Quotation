/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      HisData 
* File Name:	YLFileMapping.h 
* Programer(s):	Jerry Lee 
* Created:      20101227 
* Description:	interface for file mapping list
* History:
*******************************************************************************/

#ifndef _YLFILEMAPPING_H
#define _YLFILEMAPPING_H 

#pragma once

#include <string>
#include <vector>
#include "windows.h"
#include "datatypes.h"


using namespace std;

/*
* 文件映射类-CYLFileMapping
* 用途：该类主要用于快速读取文件
*/
class CYLFileMapping
{
public:
    CYLFileMapping();

    CYLFileMapping(const char *filename);

    ~CYLFileMapping();

    // methods
public:
    // 打开文件
    void Open(const char *filename);

    // 关闭文件
    void Close();

    // 判断当前是否有文件被打开
    bool IsOpen()
    {
        return (m_pMemory != YLNULL) && (m_handle != YLNULL);
    }

    // 是否已经处于内存映射的末端
    bool IsEof()
    {
        return (m_pCurrent-m_pMemory) >= m_fileSize;
    }

    // 是否已经处于内存映射的开端
    bool IsBof()
    {
        return m_pCurrent <= m_pMemory;
    }

    // 文件指针向前移动一段size
    void Forward(const unsigned int size)
    {
        m_pCurrent += size;
    }

    // 文件指针向后移动一段size
    void Backward(const unsigned int size)
    {
        m_pCurrent -= size;
    }

    // 重置文件指针
    void Reset()
    {
        m_pCurrent = m_pMemory;
    }

    // properties
public:
    // 得到内存指针
    char *GetMemory()
    {
        return m_pMemory;
    }

    // 得到当前指针
    char *GetCurrent()
    {
        return m_pCurrent;
    }

    // 得到整个内存映射的大小
    int GetSize()
    {
        return m_fileSize;
    }

private:
    HANDLE  m_handle;       // 文件映射句柄
    string  m_strFileName;  // 文件名
    char    *m_pMemory;     // 文件映射首地址
    char    *m_pCurrent;    // 当前指针
    int     m_fileSize;     // 文件大小
};


#endif