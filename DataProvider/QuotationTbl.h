#ifndef QUOTATIONTBL_H_HEADER_INCLUDED
#define QUOTATIONTBL_H_HEADER_INCLUDED
#include <string>
#include <map>
#include <vector>
#include "Gess.h"
using namespace std;

class CQuotationTbl
{
public:
	CQuotationTbl();
	~CQuotationTbl();

	string ToString() ;


	int SetSeqNo(const string& sInstID,unsigned int uiVal);
	int SeqNo(const string& sInstID,unsigned int& uiVal);
	unsigned int SeqNo(const string& sInstID);

	int SetDate(const string& sInstID,unsigned int uiVal);
	int Date(const string& sInstID,unsigned int& uiVal);
	unsigned int Date(const string& sInstID);
	
	int SetTime(const string& sInstID,unsigned int uiVal);
	int Time(const string& sInstID,unsigned int& uiVal);
	unsigned int Time(const string& sInstID);
	
	int SetLastClose(const string& sInstID,unsigned int uiVal);
	int LastClose(const string& sInstID,unsigned int& uiVal);
	unsigned int LastClose(const string& sInstID);
	
	int SetLastSettle(const string& sInstID,unsigned int uiVal);
	int LastSettle(const string& sInstID,unsigned int& uiVal);
	unsigned int LastSettle(const string& sInstID);
	
	int SetSettle(const string& sInstID,unsigned int uiVal);
	int Settle(const string& sInstID,unsigned int& uiVal);
	unsigned int Settle(const string& sInstID);
	
	int SetOpenPrice(const string& sInstID,unsigned int uiVal);
	int OpenPrice(const string& sInstID,unsigned int& uiVal);
	unsigned int OpenPrice(const string& sInstID);
	
	int SetHigh(const string& sInstID,unsigned int uiVal);
	int High(const string& sInstID,unsigned int& uiVal);
	unsigned int High(const string& sInstID);
	
	int SetLow(const string& sInstID,unsigned int uiVal);
	int Low(const string& sInstID,unsigned int& uiVal);
	unsigned int Low(const string& sInstID);
	
	int SetClose(const string& sInstID,unsigned int uiVal);
	int Close(const string& sInstID,unsigned int& uiVal);
	unsigned int Close(const string& sInstID);
	
	int SetAverage(const string& sInstID,unsigned int uiVal);
	int Average(const string& sInstID,unsigned int& uiVal);
	unsigned int Average(const string& sInstID);

	int SetVolume(const string& sInstID,unsigned int uiVal);
	int Volume(const string& sInstID,unsigned int& uiVal);
	unsigned int Volume(const string& sInstID);

	int SetTurnOver(const string& sInstID,unsigned int uiVal);
	int TurnOver(const string& sInstID,unsigned int& uiVal);
	unsigned int TurnOver(const string& sInstID);

	int SetPosi(const string& sInstID,unsigned int uiVal);
	int Posi(const string& sInstID,unsigned int& uiVal);
	unsigned int Posi(const string& sInstID);
	
	int SetLastPosi(const string& sInstID,unsigned int uiVal);
	int LastPosi(const string& sInstID,unsigned int& uiVal);
	unsigned int LastPosi(const string& sInstID);

	int SetAsk1(const string& sInstID,unsigned int uiVal);
	int Ask1(const string& sInstID,unsigned int& uiVal);
	unsigned int Ask1(const string& sInstID);

	int SetAskLot1(const string& sInstID,unsigned int uiVal);
	int AskLot1(const string& sInstID,unsigned int& uiVal);
	unsigned int AskLot1(const string& sInstID);

	int SetAsk2(const string& sInstID,unsigned int uiVal);
	int Ask2(const string& sInstID,unsigned int& uiVal);
	unsigned int Ask2(const string& sInstID);

	int SetAskLot2(const string& sInstID,unsigned int uiVal);
	int AskLot2(const string& sInstID,unsigned int& uiVal);
	unsigned int AskLot2(const string& sInstID);

	int SetAsk3(const string& sInstID,unsigned int uiVal);
	int Ask3(const string& sInstID,unsigned int& uiVal);
	unsigned int Ask3(const string& sInstID);

	int SetAskLot3(const string& sInstID,unsigned int uiVal);
	int AskLot3(const string& sInstID,unsigned int& uiVal);
	unsigned int AskLot3(const string& sInstID);

	int SetAsk4(const string& sInstID,unsigned int uiVal);
	int Ask4(const string& sInstID,unsigned int& uiVal);
	unsigned int Ask4(const string& sInstID);

	int SetAskLot4(const string& sInstID,unsigned int uiVal);
	int AskLot4(const string& sInstID,unsigned int& uiVal);
	unsigned int AskLot4(const string& sInstID);

	int SetAsk5(const string& sInstID,unsigned int uiVal);
	int Ask5(const string& sInstID,unsigned int& uiVal);
	unsigned int Ask5(const string& sInstID);

	int SetAskLot5(const string& sInstID,unsigned int uiVal);
	int AskLot5(const string& sInstID,unsigned int& uiVal);
	unsigned int AskLot5(const string& sInstID);

	int SetBid1(const string& sInstID,unsigned int uiVal);
	int Bid1(const string& sInstID,unsigned int& uiVal);
	unsigned int Bid1(const string& sInstID);

	int SetBidLot1(const string& sInstID,unsigned int uiVal);
	int BidLot1(const string& sInstID,unsigned int& uiVal);
	unsigned int BidLot1(const string& sInstID);

	int SetBid2(const string& sInstID,unsigned int uiVal);
	int Bid2(const string& sInstID,unsigned int& uiVal);
	unsigned int Bid2(const string& sInstID);

	int SetBidLot2(const string& sInstID,unsigned int uiVal);
	int BidLot2(const string& sInstID,unsigned int& uiVal);
	unsigned int BidLot2(const string& sInstID);

	int SetBid3(const string& sInstID,unsigned int uiVal);
	int Bid3(const string& sInstID,unsigned int& uiVal);
	unsigned int Bid3(const string& sInstID);

	int SetBidLot3(const string& sInstID,unsigned int uiVal);
	int BidLot3(const string& sInstID,unsigned int& uiVal);
	unsigned int BidLot3(const string& sInstID);
	
	int SetBid4(const string& sInstID,unsigned int uiVal);
	int Bid4(const string& sInstID,unsigned int& uiVal);
	unsigned int Bid4(const string& sInstID);
	
	int SetBidLot4(const string& sInstID,unsigned int uiVal);
	int BidLot4(const string& sInstID,unsigned int& uiVal);
	unsigned int BidLot4(const string& sInstID);

	int SetBid5(const string& sInstID,unsigned int uiVal);
	int Bid5(const string& sInstID,unsigned int& uiVal);
	unsigned int Bid5(const string& sInstID);
	
	int SetBidLot5(const string& sInstID,unsigned int uiVal);
	int BidLot5(const string& sInstID,unsigned int& uiVal);
	unsigned int BidLot5(const string& sInstID);

	int LastPrice(const string& sInstID,unsigned int uiVal);
    int SetLastPrice(const string& sInstID,unsigned int& uiVal);
    unsigned int LastPrice(const string& sInstID);

	int  SetHighLimit(const string& sInstID,unsigned int uiVal);
    int  HighLimit(const string& sInstID,unsigned int& uiVal);
    unsigned int  HighLimit(const string& sInstID);

    int  SetLowLimit(const string& sInstID,unsigned int uiVal);
	int  LowLimit(const string& sInstID,unsigned int& uiVal);
    unsigned int  LowLimit(const string& sInstID);
	
    int GetQuotation(const string& sInstID,QUOTATION& stQuotation);	
    void SetQuotation(const string& sInstID,const QUOTATION& stQuotation);
	int GetInstKeySet(char cMode,const vector<unsigned int>& vCnd, unsigned int uiNodeID, multimap<string,unsigned int>& mKeySet);

	int Init();
private:

	map<unsigned int, vector<QUOTATION*> > m_mapMarketTypeQuotation;
	map<string,QUOTATION*> m_mapQuotation;

    CGessMutex  m_mutexTbl;
};

#endif /* CQUOTATIONTBL_H_HEADER_INCLUDED_B6E38024 */
