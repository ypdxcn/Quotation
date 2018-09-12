// QuotationMgrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DataSrc.h"
#include "DataSrcDlg.h"
#include "hsstruct.h"
#include <windows.h>
#include <tlhelp32.h>
// 根据进程名称判断进程是否存在



#define WJF_MARKET_SG  0x4753   //黄金
#define WJF_MARKET_NQ  0x514e   //国内期货
#define WJF_MARKET_SF  0x4653   //国内期指
#define WJF_MARKET_HZ  0x5a48   //恒指

#define WJF_MARKET_WH  0x4857   //外汇
#define WJF_MARKET_WQ  0x5157   //外盘期货


#define WJF_MARKET_HK  0x4b48   //
#define WJF_MARKET_HK_NX 0x5048 //牛熊证
#define WJF_MARKET_HK_WR 0x5148 //权证
#define WJF_MARKET_SH  0x4853
#define WJF_MARKET_SZ  0x5a53

#define WJF_MARKET_ID  0x4449  // 国际指数
 


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
long GetLongValue3(float fValue,int nPriceUnit);
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataSrcDlg dialog

CDataSrcDlg::CDataSrcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataSrcDlg::IDD, pParent)//,m_uiCount(0)
{
	//{{AFX_DATA_INIT(CDataSrcDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	int i;
	for(i=0; i<StkBufNum; i++){
		m_Stock[i] = _T("股票: ");
	}
	m_StkPtr = 0;
	m_Min = _T("分时走势:");
	m_File = _T("文件:");

	m_uiCountNoQuote = 0;
	m_bStockDrv_Loaded = false;
}

void CDataSrcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDataSrcDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDataSrcDlg, CDialog)
	//{{AFX_MSG_MAP(CDataSrcDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SETTING, OnButtonSetting)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_MESSAGE(WM_MSG_STOCKDATA, OnStockData)
	ON_MESSAGE(WM_MSG_STOCKDLL_INIT, On_StockDll_Init)
	ON_MESSAGE(WM_MSG_STOCKDLL_RELEASE, On_StockDll_Release)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataSrcDlg message handlers
#define  TIMER_NOQUOTE   1
BOOL CDataSrcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	
	

	m_oDataSrcCpMgr.Bind((long)GetSafeHwnd());
	if (m_oDataSrcCpMgr.Init("DataSrc_wjf") == 0 && m_oDataSrcCpMgr.Start() == 0)
	{
		//#ifdef SIMULATE
		//m_oSimulator.Bind(this);
		//m_oSimulator.BeginThread();
		//#endif
		return TRUE;
	}
	else
	{
		AfxMessageBox("启动失败，请检查配置文件是否正常！");
		return FALSE;
	}
	// TODO: Add extra initialization here
}

void CDataSrcDlg::OnTimer(UINT nIDEvent)
{
	if (TIMER_NOQUOTE == nIDEvent)
	{
		if (m_bStockDrv_Loaded == true)
		{
			if (m_uiCountNoQuote > 0 && CGessDate::ThisWeekDay() != 0 && CGessDate::ThisWeekDay() != 6)
			{

				m_oDataSrcCpMgr.Logout();
			}
			m_uiCountNoQuote ++;
		}
	}
	CDialog::OnTimer(nIDEvent); 
}

void CDataSrcDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDataSrcDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
       CPaintDC dc(this); // device context for painting
	   CRect rc;	   

	   int i;
	   int k;
	   int High;
	   int y;
	   TEXTMETRIC tm;
	   char * pTitle = "  股号     名  称      最新     今开     昨收     最高     最低     成交量   成交额";
	   dc.GetTextMetrics(&tm);
	   High = tm.tmHeight + 3;
	   k = m_StkPtr;
	   y = 1;
	   dc.TextOut(1,y,pTitle,strlen(pTitle));
	   y += High;
	   for(i=0; i<StkBufNum; i++)
	   {
		   k = k % StkBufNum;
		   dc.TextOut(1,y,m_Stock[k],m_Stock[k].GetLength());
		   y += High;
		   k ++;
	   }
	   y += High;
	   dc.TextOut(1,y,m_Min,m_Min.GetLength());
	   y += High;
	   dc.TextOut(1,y,m_File,m_File.GetLength());
	   
		   
      

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDataSrcDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDataSrcDlg::OnButtonSetting() 
{
	// TODO: Add your control notification handler code here
	
}

void CDataSrcDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码

	//#ifdef SIMULATE
	//m_oSimulator.EndThread();
	//#endif
	//
	m_oDataSrcCpMgr.Stop();
	m_oDataSrcCpMgr.Finish();
}

void CDataSrcDlg::OnClose()
{
	CDialog::OnClose();
	//m_oDataSrcCpMgr.Stop();
	//m_oDataSrcCpMgr.Finish();
}
// Handles data from stock.dll
LRESULT CDataSrcDlg::OnStockData(WPARAM wParam, LPARAM lParam)				
{
    PBYTE pBuffx;
	PBYTE pFileBuf = NULL;
	RCV_DATA*	pHeader;
	PGZLXBINDATA pGZLX;
	pHeader = (RCV_DATA *) lParam;


	if (m_bStockDrv_Loaded == false)
	{
		::PostMessage(GetSafeHwnd(), WM_MSG_STOCKDLL_RELEASE, 0, 0);
	}

	m_uiCountNoQuote = 0;



	int i,j;
	int nPackSize = sizeof(RCV_DATA);
	

	switch (wParam)
	{
	case RCV_REPORT:
		{
			if (pHeader->m_nPacketNum > 0)
			{
/*				RCV_REPORT_STRUCTEx &Buf = *(PRCV_REPORT_STRUCTEx)(&pHeader->m_pReport[0]);

				pHeader->m_wDataType = wParam;
				nPackSize += pHeader->m_pReport[0].m_cbSize * pHeader->m_nPacketNum;
*/

				int nBufSize = pHeader->m_pReport[0].m_cbSize;
				PBYTE pBaseBuf = (PBYTE)&pHeader->m_pReport[0];
				for(i=0; i<pHeader->m_nPacketNum; i++)
				{
					RCV_REPORT_STRUCTEx & Buf = *(PRCV_REPORT_STRUCTEx)(pBaseBuf + nBufSize*i );
					OnNewQuotation(&Buf);
				}
			}


			int nBufSize = pHeader->m_pReport[0].m_cbSize;
			PBYTE pBaseBuf = (PBYTE)&pHeader->m_pReport[0];
			for(i=0; i<pHeader->m_nPacketNum; i++)
			{
				RCV_REPORT_STRUCTEx & Buf = *(PRCV_REPORT_STRUCTEx)(pBaseBuf + nBufSize*i );
				m_Stock[m_StkPtr].Format(" %6s %8s V=%10.0f A=%10.f N=%8.3f O=%8.3f C=%8.3f H=%8.3f L=%8.3f",
						Buf.m_szLabel,Buf.m_szName,Buf.m_fVolume,Buf.m_fAmount,Buf.m_fNewPrice,Buf.m_fOpen,Buf.m_fLastClose,\
						Buf.m_fHigh,Buf.m_fLow);
					// 增加的数据获得如下
					//TRACE("PB5=%8.3f VB5=%8.2f PS5=%8.3f VS5=%8.2f\n",Buf.m_fBuyPrice5,Buf.m_fBuyVolume5,Buf.m_fSellPrice5,Buf.m_fSellVolume5);

				m_StkPtr ++;
				m_StkPtr = m_StkPtr % StkBufNum;

			}

		}
		break;

	case RCV_FILEDATA:
		if( !pHeader->m_pData || pHeader->m_wDataType == FILE_TYPE_RES)
		{
			TRACE("MSG: CStkDemo::OnStkDataOK, Replaced data \n");
			break;
		}
		switch(pHeader->m_wDataType)
		{
		case FILE_HISTORY_EX:						// 补日线数据
		    RCV_HISTORY_STRUCTEx * pDay;
			pDay = pHeader->m_pDay;
			ASSERT(pHeader->m_pDay[0].m_head.m_dwHeadTag == EKE_HEAD_TAG);
			m_File = "日线";
			j = 0;
			for(i=0; i<pHeader->m_nPacketNum ; i++)
			{
				if( pDay[i].m_head.m_dwHeadTag == EKE_HEAD_TAG )
				{
					m_File = m_File + (const char *) pDay[i].m_head.m_szLabel + "  ";
					j ++;
				}
				if( j > 10) break;
			}
			break;

		case FILE_MINUTE_EX:						// 补分钟线数据
		/*	if (pHeader->m_nPacketNum > 0)
			{
				pHeader->m_wDataType = wParam;
				nPackSize += sizeof(RCV_MINUTE_STRUCTEx) * pHeader->m_nPacketNum;
				LPVOID lpBuffer = g_StockBuffer.PopBuffer(0, nPackSize);
				if (NULL != lpBuffer)
				{
					memcpy(lpBuffer, (const void*)lParam, nPackSize);
					g_StockBuffer.ReleaseBuffer(lpBuffer);
				}
			}		*/
			
			RCV_MINUTE_STRUCTEx * pMin;

			nPackSize = sizeof(RCV_MINUTE_STRUCTEx);
			
			pMin = pHeader->m_pMinute;
			ASSERT(pMin->m_head.m_dwHeadTag == EKE_HEAD_TAG);
			m_Min = "分时走势: ";
			for(i=0; i<pHeader->m_nPacketNum; i++)
			{
				if( pMin[i].m_head.m_dwHeadTag == EKE_HEAD_TAG )
				{
					m_Min = m_Min + (LPCSTR)pMin[i].m_head.m_szLabel + "  ";
				}
			}
			break;

		case FILE_BASE_EX:						// 钱龙兼容基本资料文件,m_szFileName仅包含文件名
			m_File.Format("基本资料: %s",pHeader->m_File.m_szFileName);
			break;

		case FILE_NEWS_EX:						// 新闻类,其类型由m_szFileName中子目录名来定
			m_File.Format("公告消息: %s",pHeader->m_File.m_szFileName);
			break;

		case FILE_SOFTWARE_EX:
			TRACE("%s\n",pHeader->m_File.m_szFileName);
			TRACE("%d\n",pHeader->m_File.m_dwLen);
			pBuffx = (PBYTE)pHeader->m_pData;
			break;

		case FILE_SHAZQDATA_EX:		//h上海国债净价交易
			pGZLX = (PGZLXBINDATA)pHeader->m_pData;
			for (i=0;i<(int)pHeader->m_File.m_dwLen /sizeof(GZLXBINDATA);i++)
			{
				TRACE("%s\n",pGZLX[i].m_szLabel);
			}
			break;
		default:
			TRACE("Msg: CStkDemo::OnStkDataOK,Unkonw data type\n");
		}
		break;
	}
	Invalidate();
	return 0L;
}

void CDataSrcDlg::OnNewQuotation(RCV_REPORT_STRUCTEx* pReport)
{
	static DWORD s_dwSeqNo = 0;
	QUOTATION stQuotationTmp = {0};

	try
	{
		time_t tmNow = pReport->m_time;
		struct tm stTime;
		localtime_r(&tmNow,&stTime);
		DWORD dwDate = (stTime.tm_year + 1900)*10000 + (stTime.tm_mon + 1)*100 + stTime.tm_mday;
		DWORD dwTime = stTime.tm_hour*10000000 + stTime.tm_min*100000 + stTime.tm_sec*1000 + 0;

		//CGessDate oDateQuo(stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday);
		//CGessTime oTimeQuo(stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
		//if (oDateQuo.CompareNow() < 0)
		//	return;
		//if (oTimeQuo.IntervalToNow() > 600)
		//	return;

		SYSTEMTIME st;
		::GetLocalTime(&st);
		//if (dwDate < st.wYear*10000 + st.wMonth*100 + st.wDay)
		//{
		//	CRLog(E_DEBUG, "日期过期:%u", dwDate);
		//	return;
		//}
			
		int nDelay = (st.wHour*3600 + st.wMinute * 60 + st.wSecond) - (stTime.tm_hour*3600 + stTime.tm_min*60 + stTime.tm_sec);
		//if (nDelay > 600 || nDelay < -600)
		//{
		//	CRLog(E_DEBUG, "(%02d:%02d:%02d-%02d:%02d:%02d)时间过期:%u", st.wHour,st.wMinute,st.wSecond,stTime.tm_hour,stTime.tm_min,stTime.tm_sec,dwTime);
		//	return;
		//}

		HSMarketDataType hsMarketType = GetMarket(pReport->m_wMarket, pReport->m_szLabel);
		if (hsMarketType == 0)
			return;

		// 将网际风黄金代码转为交易所的标准
		if (hsMarketType == (HJ_MARKET|HJ_SH_CURR))
		{
			FillHJCode(stQuotationTmp.m_CodeInfo.m_acCode, pReport->m_szLabel);
		}
		else
		{
			strncpy(stQuotationTmp.m_CodeInfo.m_acCode, pReport->m_szLabel, min(sizeof(stQuotationTmp.m_CodeInfo.m_acCode),sizeof(pReport->m_szLabel)));
		}

		int nPriceUnit = 100;
		if( WhoMarket(hsMarketType, FOREIGN_MARKET) )
			nPriceUnit = 10000;


		// 过滤掉黄金数据
		if (MakeMainMarket(hsMarketType) == (HJ_MARKET | HJ_SH_CURR))
			return;

		//strcpy(UDPPack.m_cStockName, pReport->m_szName);

		stQuotationTmp.m_CodeInfo.m_usMarketType = static_cast<unsigned int>(hsMarketType);
		stQuotationTmp.m_CodeInfo.m_usMarketType &= 0x0000FFFF;
		memcpy(stQuotationTmp.m_CodeInfo.m_acName, pReport->m_szName, min(sizeof(stQuotationTmp.m_CodeInfo.m_acName),sizeof(pReport->m_szName)));
		stQuotationTmp.m_uiSeqNo = 				static_cast<unsigned int>(s_dwSeqNo ++);                        //包号					
		stQuotationTmp.m_uiDate = 				static_cast<unsigned int>(dwDate);       					
		stQuotationTmp.m_uiTime = 				static_cast<unsigned int>(dwTime);       

		stQuotationTmp.m_uilastClose = 			static_cast<unsigned int>(GetLongValue3(pReport->m_fLastClose, nPriceUnit));  //昨收

		if (MakeMainMarket(hsMarketType) == (HJ_MARKET | HJ_SH_QH) || MakeMainMarket(hsMarketType) == (HJ_MARKET | HJ_OTHER))
		{
			stQuotationTmp.m_uiLastSettle = 		stQuotationTmp.m_uilastClose; 
		}
		else if (MakeMarket(hsMarketType) == FUTURES_MARKET || MakeMarket(hsMarketType) == WP_MARKET)
		{			
			stQuotationTmp.m_uiLastSettle = 		stQuotationTmp.m_uilastClose;  
		}
		else
			stQuotationTmp.m_uiLastSettle = 		static_cast<unsigned int>(0);                                   //昨结		



		stQuotationTmp.m_uiSettle = 			static_cast<unsigned int>(GetLongValue3(pReport->m_fAvgPrice, nPriceUnit));   //结算					
		stQuotationTmp.m_uiOpenPrice = 			static_cast<unsigned int>(GetLongValue3(pReport->m_fOpen, nPriceUnit));       //今开盘					
		stQuotationTmp.m_uiHigh = 				static_cast<unsigned int>(GetLongValue3(pReport->m_fHigh, nPriceUnit));       //最高价					
		stQuotationTmp.m_uiLow = 				static_cast<unsigned int>(GetLongValue3(pReport->m_fLow, nPriceUnit));        //最低价					
		//stQuotationTmp.m_uiClose = 				static_cast<unsigned int>(pReport->m_fLastClose * nPriceUnit);  //收盘价
		stQuotationTmp.m_uiClose = static_cast<unsigned int>(0); 
		stQuotationTmp.m_uiHighLimit = 			static_cast<unsigned int>(0);  					
		stQuotationTmp.m_uiLowLimit = 			static_cast<unsigned int>(0);	 					
		stQuotationTmp.m_uiLast = 				static_cast<unsigned int>(GetLongValue3(pReport->m_fNewPrice, nPriceUnit));   //最新价					
		stQuotationTmp.m_uiAverage = 			static_cast<unsigned int>(GetLongValue3(pReport->m_fAvgPrice, nPriceUnit));   //均价					
		stQuotationTmp.m_uiVolume = 			static_cast<unsigned int>(pReport->m_fVolume);                  //成交量					
		stQuotationTmp.m_uiWeight = 			static_cast<unsigned int>(0);                                   //成交重量					
		stQuotationTmp.m_uiTurnOver = 			static_cast<unsigned int>(pReport->m_fAmount/100);	                //成交金额
		stQuotationTmp.m_uiChiCangLiang =		static_cast<unsigned int>(pReport->m_fAmount);                  //持仓量
		stQuotationTmp.m_uiLastChiCangLiang =	static_cast<unsigned int>(0);                                   //昨日持仓量

		for (int m = 0; m < 3; m++)
		{
			stQuotationTmp.m_Bid[m].m_uiPrice = static_cast<unsigned int>(GetLongValue3(pReport->m_fBuyPrice[m], nPriceUnit));
			stQuotationTmp.m_Bid[m].m_uiVol   = static_cast<unsigned int>(pReport->m_fBuyVolume[m]);
			stQuotationTmp.m_Ask[m].m_uiPrice = static_cast<unsigned int>(GetLongValue3(pReport->m_fSellPrice[m], nPriceUnit));
			stQuotationTmp.m_Ask[m].m_uiVol   = static_cast<unsigned int>(pReport->m_fSellVolume[m]);
		}

		stQuotationTmp.m_Bid[3].m_uiPrice = static_cast<unsigned int>(GetLongValue3(pReport->m_fBuyPrice4, nPriceUnit));
		stQuotationTmp.m_Bid[3].m_uiVol   = static_cast<unsigned int>(pReport->m_fBuyVolume4);
		stQuotationTmp.m_Ask[3].m_uiPrice = static_cast<unsigned int>(GetLongValue3(pReport->m_fSellPrice4, nPriceUnit));
		stQuotationTmp.m_Ask[3].m_uiVol   = static_cast<unsigned int>(pReport->m_fSellVolume4);

		stQuotationTmp.m_Bid[4].m_uiPrice = static_cast<unsigned int>(GetLongValue3(pReport->m_fBuyPrice5, nPriceUnit));
		stQuotationTmp.m_Bid[4].m_uiVol   = static_cast<unsigned int>(pReport->m_fBuyVolume5);
		stQuotationTmp.m_Ask[4].m_uiPrice = static_cast<unsigned int>(GetLongValue3(pReport->m_fSellPrice5, nPriceUnit));
		stQuotationTmp.m_Ask[4].m_uiVol   = static_cast<unsigned int>(pReport->m_fSellVolume5);

		m_oDataSrcCpMgr.OnRecvQuotation(stQuotationTmp, nDelay);
	}
	catch(...)
	{

	}
}




DWORD CDataSrcDlg::GetMarket(WORD wMarket, const char* pszCode)
{
	switch (wMarket)
	{
	case WJF_MARKET_ID:
		return (WP_MARKET|WP_INDEX);
		break;
	case WJF_MARKET_HZ:
		return (WP_MARKET|WP_FUTURES_INDEX);
		break;
	case WJF_MARKET_SG:
		{
			if (!strncmp(pszCode, "510", 3) || !strncmp(pszCode, "511", 3))
				return (HJ_MARKET|HJ_SH_CURR);
			else if (!strncmp(pszCode, "512", 3))
				return (HJ_MARKET|HJ_WORLD);
			else
				return (HJ_MARKET|HJ_OTHER);
		}
		break;
	case WJF_MARKET_NQ:
		{
			if (!strncmp(pszCode, "0105", 4))
				return HJ_MARKET|HJ_SH_QH;
			else if (!strncmp(pszCode, "01", 2))
				return FUTURES_MARKET|SHANGHAI_BOURSE;
			if (!strncmp(pszCode, "02", 2))
				return FUTURES_MARKET|DALIAN_BOURSE;
			if (!strncmp(pszCode, "03", 2))
				return FUTURES_MARKET|ZHENGZHOU_BOURSE;


		}

		break;
	case WJF_MARKET_WH:
		if (pszCode)
		{
			if (!strncmp(pszCode, "USD", 3))
				return FOREIGN_MARKET | WH_BASE_RATE;
			else
				return FOREIGN_MARKET | WH_ACROSS_RATE;
		}

		return FOREIGN_MARKET;

		break;
	case WJF_MARKET_WQ:
		if (pszCode)
		{
			if (!strncmp(pszCode, "CHC", 3) || !strncmp(pszCode, "GLN", 3) || !strncmp(pszCode, "PAN", 3) ||
				!strncmp(pszCode, "PLN", 3) || !strncmp(pszCode, "SLN", 3))
				return (WP_MARKET | WP_COMEX);
			else if (!strncmp(pszCode, "LM", 2))
				return (WP_MARKET | WP_LME);
			else if (!strncmp(pszCode, "SBC", 3) || !strncmp(pszCode, "WHC", 3) ||
				!strncmp(pszCode, "CRC", 3) || !strncmp(pszCode, "SMC", 3) ||
				!strncmp(pszCode, "SOC", 3))
				return (WP_MARKET | WP_CBOT);
			else if (!strncmp(pszCode, "CON", 3) || !strncmp(pszCode, "HON", 3))
				return (WP_MARKET | WP_NYMEX);
			else if (!strncmp(pszCode, "GAL", 3) || !strncmp(pszCode, "OIL", 3))
				return (WP_MARKET | WP_IPE);
			else if (!strncmp(pszCode, "RBT", 3))
				return (WP_MARKET | WP_TOCOM);
			else if (!strncmp(pszCode, "CTN", 3) || !strncmp(pszCode, "SGN", 3))
				return (WP_MARKET | WP_CBOT);
			else if (!strncmp(pszCode, "0", 1))
				return (FUTURES_MARKET);
			else if (!strncmp(pszCode, "IF", 2))
				return (FUTURES_MARKET | GUZHI_BOURSE);
			else if (!strncmp(pszCode, "51", 2))
				return (WP_MARKET|WP_SICOM);
		}

		return (WP_MARKET);
		break;
	case WJF_MARKET_SF:
		return (FUTURES_MARKET | GUZHI_BOURSE);
		break;
	case WJF_MARKET_HK:
		return HK_MARKET;
		break;
	case WJF_MARKET_HK_NX:
		return HK_MARKET | NX_BOURSE;
		break;
	case WJF_MARKET_HK_WR:
		return HK_MARKET;
		break;
	case WJF_MARKET_SH:
		return STOCK_MARKET|SH_BOURSE;
		break;
	case WJF_MARKET_SZ:
		return STOCK_MARKET|SZ_BOURSE;
		break;
	default:
		return WP_MARKET;
		break;
	}
	return 0;

}

void CDataSrcDlg::FillHJCode(char* pszHJCode, const char* pszCode)
{
	if (!strcmp(pszCode, "5101"))
		strncpy(pszHJCode, "AU9999", 6);
	else if(!strcmp(pszCode, "5102"))
		strncpy(pszHJCode, "AU9995", 6);
	else if(!strcmp(pszCode, "5103"))
		strncpy(pszHJCode, "PT9995", 6);
	else if(!strcmp(pszCode, "5104"))
		strncpy(pszHJCode, "AUT5", 4);
	else if(!strcmp(pszCode, "5105"))
		strncpy(pszHJCode, "AU50", 4);
	else if(!strcmp(pszCode, "5106"))
		strncpy(pszHJCode, "AUTD", 4);
	else if(!strcmp(pszCode, "5107"))
		strncpy(pszHJCode, "AG999", 5);
	else if(!strcmp(pszCode, "5108"))
		strncpy(pszHJCode, "AGTD", 4);
	else if(!strcmp(pszCode, "5109"))
		strncpy(pszHJCode, "AU100", 5);
	else if(!strcmp(pszCode, "5110"))
		strncpy(pszHJCode, "AUTN1", 5);
	else if(!strcmp(pszCode, "5111"))
		strncpy(pszHJCode, "AUTN2", 5);
	else if(!strcmp(pszCode, "5112"))
		strncpy(pszHJCode, "AG9999", 5);
	else
		strncpy(pszHJCode, pszCode,8);
}


long GetLongValue3(float fValue,int nPriceUnit)
{
	if( fValue == 0 )
		return 0;

	if( nPriceUnit > 0 )
		return ((fValue + (float)1 / nPriceUnit * 0.1) * nPriceUnit);

	return ((fValue + (float)1 / 1 * 0.1) * 1);

}

LRESULT CDataSrcDlg::On_StockDll_Init(WPARAM wParam, LPARAM lParam)
{
	if (m_bStockDrv_Loaded == true)
		On_StockDll_Release(0,0);

	g_StockDrv.GetStockDrvInfo(RI_SUPPORTEXTHQ,NULL);	//设置 Szn Label6 Receive
	int nRes = g_StockDrv.Stock_Init(GetSafeHwnd(), WM_MSG_STOCKDATA, RCV_WORK_SENDMSG);	// 数据共享引用

	if (nRes > 0)
	{		//
		DWORD m_TV_Code = 201;
		g_StockDrv.GetStockDrvInfo(RI_ENABLETS3FILE, &m_TV_Code);

		if (1 != g_StockDrv.GetStockDrvInfo(RI_PanKouSUPPORT, NULL))
		{
			;//AfxMessageBox("不支持盘口数据传输！");
		}
		if (1 != g_StockDrv.GetStockDrvInfo(RI_DATAExtSUPPORT2, NULL))
		{
			;//AfxMessageBox("不支持扩展协议数据传输！");
		}
		m_bStockDrv_Loaded  = true;
		m_uiCountNoQuote = 0;
	}
	
	SetTimer(TIMER_NOQUOTE,15000,NULL);

	return 0;
}
LRESULT CDataSrcDlg::On_StockDll_Release(WPARAM wParam, LPARAM lParam)
{
	g_StockDrv.Stock_Quit(GetSafeHwnd());
	m_bStockDrv_Loaded = false;
	// 强制结束进程
	DWORD id = FindAppProcessID(m_oDataSrcCpMgr.GetWjfName().c_str());
	if (id != -1)
	{
		// 删除图标ICON_TRAY KENNY  20170915
		 //NOTIFYICONDATA   y_baby; 
	  //   y_baby.cbSize   =   sizeof   (NOTIFYICONDATA); 
	  //   y_baby.hWnd     =   GetSafeHwnd(); 
	  //   y_baby.uID      =   1030; 
	  //   y_baby.uFlags   =   NIF_ICON|NIF_MESSAGE|NIF_TIP; 
	  //   Shell_NotifyIcon(NIM_DELETE,   &y_baby);//图标删除

		//刷新图标
		//任务栏窗口    
	   HWND hShellTrayWnd = ::FindWindow(_T("Shell_TrayWnd"),NULL);    
	   //任务栏右边托盘图标+时间区    
	   HWND hTrayNotifyWnd = ::FindWindowEx(hShellTrayWnd,0,_T("TrayNotifyWnd"),NULL);    
	   //不同系统可能有可能没有这层    
	   HWND hSysPager = ::FindWindowEx(hTrayNotifyWnd,0,_T("SysPager"),NULL);    
	   //托盘图标窗口    
	   HWND hToolbarWindow32;    
	   if (hSysPager)  
	   {    
		   hToolbarWindow32 = ::FindWindowEx(hSysPager,0,_T("ToolbarWindow32"),NULL);    
	   }    
	   else  
	   {    
		   hToolbarWindow32 = ::FindWindowEx(hTrayNotifyWnd,0,_T("ToolbarWindow32"),NULL);    
	   }    
	   if (hToolbarWindow32)  
	   {    
		   RECT rect_bar;    
		   ::GetWindowRect(hToolbarWindow32,&rect_bar);    
		   int width = rect_bar.right - rect_bar.left;    
		   int height = rect_bar.bottom - rect_bar.top;    
		   //从任务栏中间从左到右 MOUSEMOVE一遍，所有图标状态会被更新    
		   for (int x = 1; x<width; x++)  {    
			   ::PostMessage(hToolbarWindow32,WM_MOUSEMOVE,0,MAKELPARAM(x,height/2));    
		   }    
	   }    
      //end刷新图标

		// 关闭当前实例
		HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS,FALSE, id);
		if(ProcessHandle)	
			TerminateProcess(ProcessHandle, 0);
	}

	KillTimer(TIMER_NOQUOTE);
	return 0;
}

DWORD CDataSrcDlg::FindAppProcessID(string szAppName)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32); 
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	// toLower
	transform(szAppName.begin(), szAppName.end(), szAppName.begin(), tolower);

	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	string szSystemApp;

	while(bMore)
	{
		szSystemApp = pe32.szExeFile;
		transform(szSystemApp.begin(), szSystemApp.end(), szSystemApp.begin(), tolower);
		if (0 == szSystemApp.compare(0, szSystemApp.length(), szAppName))
		{
			::CloseHandle(hProcessSnap);
			return pe32.th32ProcessID;
		}
		bMore = ::Process32Next(hProcessSnap, &pe32);
	}
	::CloseHandle(hProcessSnap);


	return -1;
}

//#define	WM_MSG_STOCKDATA	WM_APP + 1
//#define RCV_REPORT			0x3f001234
//#define RCV_FILEDATA		0x3f001235
//#define RCV_PANKOUDATA      0x3f001258
//int CDataSrcDlg::SimulateMsg()
//{
//	try
//	{
//		if (m_uiCount > 1000000)
//			return -1;
//
//		char szInstID[] = "Au(T+D)";
//
//		SYSTEMTIME sysTime;
//		GetLocalTime(&sysTime);
//		DWORD dwDate = sysTime.wYear*10000 + sysTime.wMonth*100 + sysTime.wDay;
//		DWORD dwTime = sysTime.wHour*10000000 + sysTime.wMinute*100000 + sysTime.wSecond*1000 + sysTime.wMilliseconds ;
//
//		RCV_DATA* pstData = new RCV_DATA;
//		memset(pstData, 0x00, sizeof(RCV_DATA));
//		pstData->m_nPacketNum = 1;
//		pstData->m_pReport = new RCV_REPORT_STRUCTEx;
//		pstData->m_pReport->m_cbSize = sizeof(RCV_REPORT_STRUCTEx);
//		pstData->m_pReport->m_time = dwTime;
//		memcpy(pstData->m_pReport->m_szName, szInstID, strlen(szInstID));
//		memcpy(pstData->m_pReport->m_szLabel, szInstID, strlen(szInstID));
//
//
//		WPARAM wParam = RCV_REPORT;
//		LPARAM lParam = (LPARAM)(pstData);
//
//		::SendMessage(this->m_hWnd, WM_MSG_STOCKDATA, wParam, lParam);
//
//		delete pstData->m_pReport;
//		delete pstData;
//
//		m_uiCount++;
//		return 0;
//	}
//	catch(...)
//	{
//		return -1;
//	}
//}