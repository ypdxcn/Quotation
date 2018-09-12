/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      convergence 
* File Name:	YLFileMapping.cpp 
* Programer(s):	Jerry Lee 
* Created:      20101227 
* Description:	implementation of file mapping list
* History:
*******************************************************************************/



#include "YLFileMapping.h"
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

int getfilesize(const char *filename)
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

CYLFileMapping::CYLFileMapping()
:m_handle(0),
m_fileSize(0),
m_pMemory(YLNULL),
m_pCurrent(YLNULL)
{

}

CYLFileMapping::CYLFileMapping(const char *filename)
:m_handle(0),
m_fileSize(0),
m_pMemory(YLNULL),
m_pCurrent(YLNULL)
{
    Open(filename);
}


CYLFileMapping::~CYLFileMapping()
{
    Close();
}

//
void CYLFileMapping::Open(const char *filename)
{
    m_strFileName = filename;

    unsigned int mapFlag;

	// modified by Ben 20110401
    HANDLE hFile = ::CreateFile(m_strFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE /*FILE_SHARE_READ*/,
        NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	// end mod

    m_fileSize = getfilesize(m_strFileName.c_str());

    if (m_fileSize <= 0)
    {
        return;
    }

    m_handle = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    ::CloseHandle(hFile);

    mapFlag = FILE_MAP_READ;

    if ((m_handle != 0) && (GetLastError() != ERROR_ALREADY_EXISTS))
    {
        m_pMemory = (char *)::MapViewOfFile(m_handle, mapFlag, 0, 0, 0);

        if (m_pMemory != YLNULL)
        {
            m_pCurrent = m_pMemory;
        }
        else
        {
            // throw 
        }
    }
    else
    {
        // throw
    }
}

//
void CYLFileMapping::Close()
{
    if (m_pMemory != YLNULL)
    {
        UnmapViewOfFile(m_pMemory);
        m_pMemory = YLNULL;
    }

    if (m_handle != YLNULL)
    {
        CloseHandle(m_handle);
        m_handle = YLNULL;
    }
}

