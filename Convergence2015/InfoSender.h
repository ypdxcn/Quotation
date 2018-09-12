/*******************************************************************************
* (C) Copyright 2010 Ylink Computer System LTD 
* 
* These computer program listings and specifications are the property of Ylink 
* LTD and shall not be reproduced or copied or used in whole or in 
* part without written permission from Giant Electronics LTD .
*
* Project:      cvg 
* File Name:	InfoSender.h 
* Programer(s):	Jerry Lee 
* Created:      20110228 
* Description:	interface for sending information
* History:
*******************************************************************************/

#pragma once

#include "XmlInfoIndexFile.h"
#include <string>
#include "DeliverMgr.h"

using namespace std;

class CInfoSender
{
public:
    CInfoSender(const char *contentPath, CDeliverMgr *pDeliverMgr,
        unsigned int nodeId = MAX_UINT);

	CInfoSender(CDeliverMgr *pDeliverMgr, unsigned int nodeId = MAX_UINT);

    ~CInfoSender();

public:
    // 发送一条资讯
    void Send(const InfoFile *pIF);

   /**
	* 发送一条资讯
	*/
    void Send();

	void SendPacket(const InfoData &infoData);
private:
    string m_contentPath;

    char m_buf[8192];

    CDeliverMgr *m_pDeliverMgr;

    unsigned int m_nodeId;
};
