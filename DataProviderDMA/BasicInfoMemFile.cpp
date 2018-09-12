#include "BasicInfoMemFile.h"
#include "strutils.h"
#include "Logger.h"
#include <fstream>

using namespace std;
using namespace strutils;

CBasicInfMemFile::CBasicInfMemFile()
:m_pBasicInf(0)
{
}

CBasicInfMemFile::~CBasicInfMemFile(void)
{
	m_mapInstState.clear();

}

string CBasicInfMemFile::ToString()
{	
	string sRtn = "Date=";
	sRtn += strutils::ToString<unsigned int>(m_pBasicInf->nDate);
	sRtn += "\r\nCount=";
	sRtn += strutils::ToString<unsigned int>(m_pBasicInf->nCount);
	
	for (int i = 0; i < min(m_pBasicInf->nCount,MAX_INST_NUMBER); i++)
	{		
		sRtn += "\r\n";
		sRtn += m_pBasicInf->aInstState[i].instID;
		sRtn += "=";
		sRtn.append(1, m_pBasicInf->aInstState[i].tradeState);
	}
	return sRtn;
}

BOOL CBasicInfMemFile::Create(LPCTSTR lpszFileName)
{
#if 1
	size_t nSize = sizeof(BASIC_INF);
	BOOL blRtn = MapFile(lpszFileName);//,FALSE, 0, FALSE, FALSE, FALSE, 0, nSize);
	if (TRUE == blRtn)
	{
		m_pBasicInf = (BASIC_INF*)m_lpData;
		if (0 == m_pBasicInf)
			return FALSE;

		for (int i = 0; i < m_pBasicInf->nCount && i < MAX_INST_NUMBER; i++)
		{
			INST_INFO* p = &m_pBasicInf->aInstState[i];
			string sInst = p->instID;
			if (sInst == "")
				continue;

			p->uiClose = 0;
			m_mapInstState[sInst] = p;
		}
	}
	return blRtn;
#else
	ofstream ofs;
	ofs.open(lpszFileName, ios::binary | ios::out | ios::trunc);

	BASIC_INF oInf;
	oInf.nDate = 20131030;
	oInf.nCount = 13;

	INST_INFO pInst[MAX_INST_NUMBER] = 
	{
		{"Ag(T+D)", 'd', '5', 0, 0},
		{"MAu(T+D)", 'd', '0', 0, 0},
		{"Au(T+D)", 'd', '5', 0, 0},
		{"Au99.99", 's', '5', 0, 0},
		{"Au99.95", 's', '5', 0, 0},
		{"Au100g", 's', '5', 0, 0},
		{"Pt99.95", 's', '5', 0, 0},
		{"Au99.5", 's', '5', 0, 0},
		{"Au50g", 's', '5', 0, 0},
		{"Ag99.9", 'f', '5', 0, 0},
		{"Ag99.99", 'f', '5', 0, 0},
		{"Au(T+N1)", 'd', '5', 0, 0},
		{"Au(T+N2)", 'd', '5', 0, 0}
	};

	for (int i = 0; i < min(oInf.nCount, MAX_INST_NUMBER); i++)
	{
		strcpy(oInf.aInstState[i].instID, pInst[i].instID);
		oInf.aInstState[i].marketID = pInst[i].marketID;
		oInf.aInstState[i].tradeState = pInst[i].tradeState;
		oInf.aInstState[i].uiSeqNo = pInst[i].uiSeqNo;
		oInf.aInstState[i].uiClose = 0;
	}
	ofs.write((const char*)(&oInf), sizeof(oInf));
	ofs.close();
	return TRUE;
#endif
}

void CBasicInfMemFile::Close()
{
	UnMap();
}

BOOL CBasicInfMemFile::GetInstState(const string& sInst, char& cState)
{
	BOOL blRtn = FALSE;
	map<string, INST_INFO*>::iterator it = m_mapInstState.find(sInst);
	if (it != m_mapInstState.end())
	{
		cState = (*it).second->tradeState;
		blRtn = TRUE;
	}
	return blRtn;
}

BOOL CBasicInfMemFile::SetInstState(const string& sInst, char cState, char cMarketID)
{
	BOOL blRtn = FALSE;
	map<string, INST_INFO*>::iterator it = m_mapInstState.find(sInst);
	if (it != m_mapInstState.end())
	{
		(*it).second->tradeState = cState;
		if (I_END == cState)
		{
			(*it).second->uiSeqNo = 0;
		}
		blRtn = TRUE;

		Flush();
	}
	return blRtn;
}

int CBasicInfMemFile::SwitchTradeDate(unsigned int uiDate)
{
	int nRtn = -1;
	if (0 != m_pBasicInf && m_pBasicInf->nDate != uiDate)
	{
		//«–ªªΩª“◊»’£¨–Ú∫≈«Â¡„
		CRLog(E_APPINFO, "%s", "«–ªªΩª“◊»’£¨–Ú∫≈«Â¡„°£");
		for (map<string, INST_INFO*>::iterator it = m_mapInstState.begin(); it != m_mapInstState.end(); ++it)
		{
			(*it).second->uiSeqNo = 0;
			(*it).second->uiClose = 0;
		}

		m_pBasicInf->nDate = uiDate;
		nRtn = 0;
	}
	return nRtn;
}

unsigned int CBasicInfMemFile::TradeDate()
{
	if (0 != m_pBasicInf)
	{
		return m_pBasicInf->nDate;
	}
	else
	{
		return 0;
	}
}

int CBasicInfMemFile::IsSeqNo(const string& sInst, unsigned int uiSeqNo,unsigned int uiClose)
{
	int nRtn = 0;
	map<string, INST_INFO*>::iterator it = m_mapInstState.find(sInst);
	if (it != m_mapInstState.end())
	{
		INST_INFO* pInstState = (*it).second;		
		if ( (MARKET_DEFER == pInstState->marketID && (I_NORMAL == pInstState->tradeState || I_GRP_ORDER == pInstState->tradeState || I_DERY_APP == pInstState->tradeState))
		  || (MARKET_SPOT == pInstState->marketID && I_NORMAL == pInstState->tradeState)
		  || (MARKET_FORWARD == pInstState->marketID && (I_NORMAL == pInstState->tradeState || I_GRP_ORDER == pInstState->tradeState))
		   )
		{
			//–Ú∫≈≈–∂œ
			if (uiSeqNo > pInstState->uiSeqNo)
			{
				pInstState->uiSeqNo = uiSeqNo;
			}
			else
			{
				nRtn = -1;
			}
		}
		else
		{
			if (0 == pInstState->uiClose && 0 != uiClose)
			{
				pInstState->uiClose = uiClose;
				nRtn = -3;
			}
			else
			{
				nRtn = -2;
			}
		}
	}
	return nRtn;
}