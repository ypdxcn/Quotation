#include <iostream>
#include "osdepend.h"
#include <string>
#include "SamplerCpMgr.h"
#include "Encode.h"
#include <cmath>
#include "BroadcastPacket.h"



CSamplerCpMgr theMgr;



BOOL WINAPI ConsoleHandler(DWORD dwEvent)
{
	switch ( dwEvent )
	{
	case CTRL_C_EVENT:
		break;
	case CTRL_BREAK_EVENT:
		break;
	case CTRL_CLOSE_EVENT:
		theMgr.HandleConsolMsg(dwEvent);
		break;
	case CTRL_LOGOFF_EVENT:
		
		break;
	case CTRL_SHUTDOWN_EVENT:
		theMgr.HandleConsolMsg(dwEvent);
		break;
	}
	return TRUE;
}
HWND g_hManagerWnd = NULL;
#define AllProgramShareMsg_str ("AllProgramShareMsg")
#define ShareMsgwParam_ProcessSucc   	0x0002	 // 启动进程成功
#define Section_This				 "本文件信息"
#define Entry_CurProcessID			 "当前进程ID值"
#define Entry_CurProcessID_Child	 "当前子进程ID值"
#define Entry_CurProcessIDLastTime	 "当前进程最后一次相应时间"
#define Entry_CurProcessIDWnd		 "当前进程窗口句柄"

int main(int argc, char* argv[])
{
	try
	{

		char szFileName[_MAX_PATH], szFilePath[_MAX_PATH];
		char * pcName;
		::GetModuleFileName(0,szFileName, _MAX_PATH);
		::GetFullPathName(szFileName, _MAX_PATH, szFilePath, &pcName);
		char szBuf[_MAX_PATH];
		strcpy(szBuf, pcName);
		*pcName = '\0';
		SetCurrentDirectory(szFilePath);

#ifndef _DEBUG
		if (argc < 3)
		{
			cout << "Unable to run without programer!" << endl;
			CRLog(E_ERROR,"Unable to run without programer!");
			return -1;
		}
		g_hManagerWnd = (HWND)atol(&(argv[2][1]));

		if( g_hManagerWnd )
			::PostMessage(g_hManagerWnd,RegisterWindowMessage(AllProgramShareMsg_str),ShareMsgwParam_ProcessSucc,0);

		char acProfile[256];
		sprintf(acProfile, "%sCOLLECTER_SETTING\\%s.dyn",szFilePath ,&(argv[1][1]));


		DWORD dwProcessId = -1;

		char acKeyValue[256];

		if (GetPrivateProfileString(Section_This, Entry_CurProcessID, "-1", acKeyValue, 256, acProfile))
		{
			dwProcessId = atol(acKeyValue);
		}
		if( dwProcessId != (DWORD)-1 )
		{	
			HANDLE hProcess = ::OpenProcess( PROCESS_TERMINATE, FALSE, dwProcessId );
			if( hProcess != NULL ) {
				if ( ::TerminateProcess( hProcess, 0 ) ) {
					::CloseHandle( hProcess );
					return TRUE;
				}
				else {
					::CloseHandle( hProcess );
				}
			}
		}


		// save pid
		dwProcessId = GetCurrentProcessId();
		sprintf(acKeyValue, "%d", dwProcessId);
		WritePrivateProfileString(Section_This, Entry_CurProcessID, acKeyValue, acProfile);


		// save handle		
		//sprintf(acKeyValue, "%d", GetSafeHwnd());
		//WritePrivateProfileString(Section_This, Entry_CurProcessIDWnd, acKeyValue, g_strProfile);

		time_t tm = time(NULL);
		sprintf(acKeyValue, "%d", tm);
		WritePrivateProfileString(Section_This, Entry_CurProcessIDLastTime, acKeyValue, acProfile);
#endif
		
		char * pcTmp = szBuf + strlen(szBuf) - 1;
		for ( ; pcTmp >= szBuf; pcTmp-- )
		{
			if ( *pcTmp == '.' )
			{
				*pcTmp = '\0';
				break;
			}
		}

		std::string sProcName = szBuf;
		
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != NO_ERROR)
		{
			CRLog(E_ERROR,"Error at WSAStartup()");
			return -1;
		}

		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
		{
			cout << "Unable to install console msg handler!" << endl;
		}

		if (0 == theMgr.Init(sProcName))
		{
			theMgr.Start();
			theMgr.StartMe();
			theMgr.Run();
			theMgr.StopMe();
			theMgr.Stop();
			theMgr.Finish();

			
			//退出时未作所有线程同步，简单sleep
			msleep(1);
		}

		WSACleanup();
		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception");
		return -1;
	}
}


