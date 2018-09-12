// QuotationMgr.h : main header file for the QUOTATIONMGR application
//

#if !defined(AFX_DATASRC_H__6887D1FD_5E11_4ED6_827B_F169628A6EE9__INCLUDED_)
#define AFX_DATASRC_H__6887D1FD_5E11_4ED6_827B_F169628A6EE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDataSrcApp:
// See DataSrcApp.cpp for the implementation of this class
//

class CDataSrcApp : public CWinApp
{
public:
	CDataSrcApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataSrcApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDataSrcApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUOTATIONMGR_H__6887D1FD_5E11_4ED6_827B_F169628A6EE9__INCLUDED_)
