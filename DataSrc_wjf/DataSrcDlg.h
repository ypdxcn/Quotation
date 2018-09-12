// DataSrcDlg.h : header file
//

#if !defined(AFX_DATASRCDLG_H__40959124_474D_4F0E_A9FE_CA0DD5FEF9DD__INCLUDED_)
#define AFX_DATASRCDLG_H__40959124_474D_4F0E_A9FE_CA0DD5FEF9DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataSrcCpMgr.h"
#include "WorkThread.h"

#include "Stockdrv.H"
#include "STKDRV.H"

extern CSTKDRV g_StockDrv;


/////////////////////////////////////////////////////////////////////////////
// CDataSrcDlg dialog

class CDataSrcDlg : public CDialog
{

// Construction
public:
	CDataSrcDlg(CWnd* pParent = NULL);	// standard constructor

	enum {StkBufNum = 8};
	// Data
public:
	CString m_Stock[StkBufNum];		// 股票数据
	int		m_StkPtr;				// 数据指针
	CString	m_Min;					// 分时数据
	CString	m_File;					// 文件数据


// Dialog Data
	//{{AFX_DATA(CDataSrcDlg)
	enum { IDD = IDD_DATASRC_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataSrcDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDataSrcDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonSetting();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg LRESULT OnStockData(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT On_StockDll_Init(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_StockDll_Release(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();

private:
	CDataSrcCpMgr m_oDataSrcCpMgr;
	unsigned long m_uiCountNoQuote;			//超时无来实时报价次数  added by Ben 20110429
	bool m_bStockDrv_Loaded;
	
	
private:	
	void OnNewQuotation(RCV_REPORT_STRUCTEx* pReport);
	DWORD GetMarket(WORD wMarket, const char* pszCode);
	void FillHJCode(char* pszHJCode, const char* pszCode);



	DWORD FindAppProcessID(string szAppName);


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUOTATIONMGRDLG_H__40959124_474D_4F0E_A9FE_CA0DD5FEF9DD__INCLUDED_)
