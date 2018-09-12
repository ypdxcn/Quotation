// STKDRV.h: interface for the CSTKDRV class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STKDRV_H__4BE51F0E_A261_11D2_B30C_00C04FCCA334__INCLUDED_)
#define AFX_STKDRV_H__4BE51F0E_A261_11D2_B30C_00C04FCCA334__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	 WM_MSG_STOCKDATA			    WM_APP + 101
#define	 WM_MSG_STOCKDLL_INIT			WM_APP + 102
#define	 WM_MSG_STOCKDLL_RELEASE		WM_APP + 103

class CSTKDRV  
{
public:
	CSTKDRV();
	virtual ~CSTKDRV();
public:
	DWORD GetStockDrvInfo(int nInfo,void * pBuf);
	int SetupReceiver(BOOL bSetup);
	int GetTotalNumber();
	int Stock_Quit(HWND hWnd);
	int Stock_Init(HWND hWnd,UINT uMsg,int nWorkMode);
	int (WINAPI *	m_pfnStock_Init)(HWND hWnd,UINT Msg,int nWorkMode);
	int (WINAPI *	m_pfnStock_Quit)(HWND hWnd);
	int (WINAPI *	m_pfnGetTotalNumber)();													
	int	(WINAPI *	m_pfnSetupReceiver)(BOOL bSetup);
	DWORD (WINAPI * m_pfnGetStockDrvInfo)(int nInfo,void * pBuf);
private:
	void GetAdress();
	HINSTANCE	m_hSTKDrv;
};

#endif // !defined(AFX_STKDRV_H__4BE51F0E_A261_11D2_B30C_00C04FCCA334__INCLUDED_)
