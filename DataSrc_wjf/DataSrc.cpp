// QuotationMgr.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DataSrc.h"
#include "DataSrcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataSrcApp

BEGIN_MESSAGE_MAP(CDataSrcApp, CWinApp)
	//{{AFX_MSG_MAP(CDataSrcApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataSrcApp construction

CDataSrcApp::CDataSrcApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDataSrcApp object

CDataSrcApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDataSrcApp initialization

BOOL CDataSrcApp::InitInstance()
{


	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	char szFileName[_MAX_PATH], szFilePath[_MAX_PATH];
	char * pcName;
	::GetModuleFileName(0,szFileName, _MAX_PATH);
	::GetFullPathName(szFileName, _MAX_PATH, szFilePath, &pcName);
	char szBuf[_MAX_PATH];
	strcpy(szBuf, pcName);
	*pcName = '\0';
	SetCurrentDirectory(szFilePath);

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
		//CRLog(E_ERROR,"Error at WSAStartup()");
		return -1;
	}


	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CDataSrcDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	WSACleanup();
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
