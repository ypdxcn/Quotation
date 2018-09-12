#include <sstream>
#include <iomanip>
#include "SamplerPacket.h"
#include "QuotationTbl.h"

CQuotationTbl::CQuotationTbl()
{

}

CQuotationTbl::~CQuotationTbl()
{
	for (map<string,QUOTATION*>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
	{
		delete (*it).second;
	}
	m_mapQuotation.clear();


	for (map<unsigned int, vector<QUOTATION*> >::iterator itMt = m_mapMarketTypeQuotation.begin(); itMt != m_mapMarketTypeQuotation.end(); ++itMt)
	{
		(*itMt).second.clear();
	}
	m_mapMarketTypeQuotation.clear();

}

string CQuotationTbl::ToString() 
{
	std::stringstream ss;

	
	return ss.str();
}

int CQuotationTbl::SetSeqNo(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiSeqNo = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::SeqNo(const string& sInstID,unsigned int& uiVal)
{
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiSeqNo;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::SeqNo(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiSeqNo;
					}
		return uiVal;
}


int CQuotationTbl::SetDate(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiDate = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Date(const string& sInstID,unsigned int& uiVal)
{
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiDate;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Date(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiDate;
					}
		return uiVal;
}


int CQuotationTbl::SetTime(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiTime = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Time(const string& sInstID,unsigned int& uiVal)
{
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiTime;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Time(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiTime;
					}
		return uiVal;
}


int CQuotationTbl::SetLastClose(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uilastClose = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::LastClose(const string& sInstID,unsigned int& uiVal)
{
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uilastClose;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::LastClose(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uilastClose;
					}
		return uiVal;
}


int CQuotationTbl::SetLastSettle(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiLastSettle = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::LastSettle(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLastSettle;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::LastSettle(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLastSettle;
					}
		return uiVal;
}


int CQuotationTbl::SetSettle(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiSettle = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Settle(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiSettle;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Settle(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiSettle;
					}
		return uiVal;
}


int CQuotationTbl::SetOpenPrice(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiOpenPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::OpenPrice(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiOpenPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::OpenPrice(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiOpenPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetHigh(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiHigh = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::High(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiHigh;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::High(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiHigh;
					}
		return uiVal;
}


int CQuotationTbl::SetLow(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiLow = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Low(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLow;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Low(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLow;
					}
		return uiVal;
}


int CQuotationTbl::SetClose(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiClose = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Close(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiClose;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Close(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiClose;
					}
		return uiVal;
}


int CQuotationTbl:: SetHighLimit(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiHighLimit = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl:: HighLimit(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiHighLimit;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl:: HighLimit(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiHighLimit;
					}
		return uiVal;
}


int CQuotationTbl:: SetLowLimit(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiLowLimit = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl:: LowLimit(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLowLimit;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl:: LowLimit(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLowLimit;
					}
		return uiVal;
}

int CQuotationTbl::SetLastPrice(const string& sInstID,unsigned int& uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiLast = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::LastPrice(const string& sInstID,unsigned int uiVal)
{
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLast;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::LastPrice(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLast;
					}
		return uiVal;
}


int CQuotationTbl::SetAverage(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiAverage = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Average(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiAverage;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Average(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiAverage;
					}
		return uiVal;
}


int CQuotationTbl::SetVolume(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiVolume = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Volume(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiVolume;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Volume(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiVolume;
					}
		return uiVal;
}


int CQuotationTbl::SetTurnOver(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiTurnOver = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::TurnOver(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiTurnOver;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::TurnOver(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiTurnOver;
					}
		return uiVal;
}


int CQuotationTbl::SetPosi(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiChiCangLiang = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Posi(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiChiCangLiang;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Posi(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiChiCangLiang;
					}
		return uiVal;
}


int CQuotationTbl::SetLastPosi(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_uiLastChiCangLiang = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::LastPosi(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLastChiCangLiang;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::LastPosi(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_uiLastChiCangLiang;
					}
		return uiVal;
}


int CQuotationTbl::SetAsk1(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[0].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Ask1(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[0].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Ask1(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[0].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetAskLot1(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[0].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::AskLot1(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[0].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::AskLot1(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[0].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetAsk2(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[1].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Ask2(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[1].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Ask2(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[1].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetAskLot2(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[1].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::AskLot2(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[1].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::AskLot2(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[1].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetAsk3(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[2].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Ask3(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[2].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Ask3(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[2].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetAskLot3(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[2].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::AskLot3(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[2].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::AskLot3(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[2].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetAsk4(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[3].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Ask4(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[3].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Ask4(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[3].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetAskLot4(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[3].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::AskLot4(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[3].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::AskLot4(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[3].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetAsk5(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[4].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Ask5(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[4].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Ask5(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[4].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetAskLot5(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Ask[4].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::AskLot5(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[4].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::AskLot5(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Ask[4].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetBid1(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[0].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Bid1(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[0].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Bid1(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[0].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetBidLot1(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[0].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::BidLot1(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[0].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::BidLot1(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[0].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetBid2(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[1].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Bid2(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[1].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Bid2(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[1].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetBidLot2(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[1].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::BidLot2(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[1].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::BidLot2(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[1].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetBid3(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[2].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Bid3(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[2].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Bid3(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[2].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetBidLot3(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[2].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::BidLot3(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[2].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::BidLot3(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[2].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetBid4(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[3].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Bid4(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[3].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Bid4(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[3].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetBidLot4(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[3].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::BidLot4(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[3].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::BidLot4(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[3].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::SetBid5(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[4].m_uiPrice = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::Bid5(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[4].m_uiPrice;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::Bid5(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[4].m_uiPrice;
					}
		return uiVal;
}


int CQuotationTbl::SetBidLot5(const string& sInstID,unsigned int uiVal)
{
    int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			(*it).second->m_Bid[4].m_uiVol = uiVal;
			nRtn = 0;
		}
		return nRtn;
}

int CQuotationTbl::BidLot5(const string& sInstID,unsigned int& uiVal)
{	
		int nRtn = -1;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[4].m_uiVol;
			nRtn = 0;
		}
		return nRtn;
}

unsigned int CQuotationTbl::BidLot5(const string& sInstID)
{
    unsigned int uiVal = 0xFFFFFFFF;
		CGessGuard guard(m_mutexTbl);

		map<string,QUOTATION*>::iterator it = m_mapQuotation.find(sInstID);
		if (it != m_mapQuotation.end())
		{
			uiVal = (*it).second->m_Bid[4].m_uiVol;
					}
		return uiVal;
}


int CQuotationTbl::GetQuotation(const string& sInstID,QUOTATION& stQuotation)
{
	int nRtn = -1;
	CGessGuard guard(m_mutexTbl);

	map<string,QUOTATION*>::const_iterator it = m_mapQuotation.find(sInstID);
	if (it != m_mapQuotation.end())
	{
		stQuotation = *((*it).second);
		nRtn = 0;
	}
	return nRtn;
}

void CQuotationTbl::SetQuotation(const string& sInstID, const QUOTATION& stQuotation)
{
	CGessGuard guard(m_mutexTbl);

	map<string,QUOTATION*>::const_iterator it = m_mapQuotation.find(sInstID);
	if (it == m_mapQuotation.end())
	{
		QUOTATION * p = new QUOTATION;
		*p = stQuotation;
		m_mapQuotation[sInstID] = p;

		unsigned int uiMarketType = stQuotation.m_CodeInfo.m_usMarketType;
		map<unsigned int, vector<QUOTATION*> >::iterator itMt = m_mapMarketTypeQuotation.find(uiMarketType);
		if (itMt != m_mapMarketTypeQuotation.end())
		{
			(*itMt).second.push_back(p);
		}
		else
		{
			vector<QUOTATION*> v;
			v.push_back(p);
			m_mapMarketTypeQuotation[uiMarketType] = v;
		}
	}
	else
	{
		*((*it).second) = stQuotation;
	}
	return;
}

int CQuotationTbl::GetInstKeySet(char cMode,const vector<unsigned int>& vCnd, unsigned int uiNodeID, multimap<string,unsigned int>& mKeySet)
{
	CGessGuard guard(m_mutexTbl);

	int nRtn = -1;
	if (vCnd.size() == 1 && vCnd[0] == 0xFFFFFFFF)
	{
		for (map<string,QUOTATION*>::iterator it =  m_mapQuotation.begin(); it !=  m_mapQuotation.end(); ++it)
		{
			mKeySet.insert(pair<string,unsigned int>((*it).first, uiNodeID));
		}
	}
	else
	{
		if (cMode = '0')
		{
			for (vector<unsigned int>::const_iterator it = vCnd.begin(); it != vCnd.end(); ++it)
			{
				map<unsigned int, vector<QUOTATION*> >::iterator itMt = m_mapMarketTypeQuotation.find(*it);
				if (itMt != m_mapMarketTypeQuotation.end())
				{
					for (vector<QUOTATION*>::iterator itIndex = (*itMt).second.begin(); itIndex != (*itMt).second.end(); ++itIndex)
					{
						mKeySet.insert(pair<string,unsigned int>((*itIndex)->m_CodeInfo.m_acCode, uiNodeID));
					}

					nRtn = 0;
				}
			}
		}
		else if (cMode = '1')
		{
			for (vector<unsigned int>::const_iterator it = vCnd.begin(); it != vCnd.end(); ++it)
			{
				for (map<unsigned int, vector<QUOTATION*> >::iterator itMt = m_mapMarketTypeQuotation.begin(); itMt != m_mapMarketTypeQuotation.end(); ++itMt)
				{
					if ((*itMt).first  != *it)
					{
						for (vector<QUOTATION*>::iterator itIndex = (*itMt).second.begin(); itIndex != (*itMt).second.end(); ++itIndex)
						{
							mKeySet.insert(pair<string,unsigned int>((*itIndex)->m_CodeInfo.m_acCode, uiNodeID));
						}
						nRtn = 0;
					}
				}
			}
		}
		else
		{

		}
	}
	return nRtn;
}

//##ModelId=4916CB8F0196
int CQuotationTbl::Init()
{
	CGessGuard guard(m_mutexTbl);

	for (map<string,QUOTATION*>::iterator it = m_mapQuotation.begin(); it != m_mapQuotation.end(); ++it)
	{
		delete (*it).second;
	}
	m_mapQuotation.clear();


	for (map<unsigned int, vector<QUOTATION*> >::iterator itMt = m_mapMarketTypeQuotation.begin(); itMt != m_mapMarketTypeQuotation.end(); ++itMt)
	{
		(*itMt).second.clear();
	}
	m_mapMarketTypeQuotation.clear();
	return 0;
}