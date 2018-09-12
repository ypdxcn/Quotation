#include <iostream>
#include "osdepend.h"
#include <string>
#include "NmCpMgr.h"
#include <signal.h>

CNmCpMgr theMgr;

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD dwEvent)
{
	switch ( dwEvent )
	{
	case CTRL_C_EVENT:
		break;
	case CTRL_BREAK_EVENT:
		break;
	case CTRL_CLOSE_EVENT:
		break;
	case CTRL_LOGOFF_EVENT:
		
		break;
	case CTRL_SHUTDOWN_EVENT:
		break;
	}
	return TRUE;
}
#endif

int main(int argc, char* argv[])
{
	try
	{
		std::string sProcName = "NmMon";
#ifdef WIN32
		char szFileName[_MAX_PATH], szFilePath[_MAX_PATH];
		char * pcName;
		::GetModuleFileName(0,szFileName, _MAX_PATH);
		::GetFullPathName(szFileName, _MAX_PATH, szFilePath, &pcName);
		char szBuf[_MAX_PATH];
		strcpy(szBuf, pcName);
		*pcName = '\0';
		SetCurrentDirectory(szFilePath);
		
		char * pcTmp = szBuf + strlen(szBuf) - 1;
		for ( ; pcTmp >= szBuf; pcTmp-- )
		{
			if ( *pcTmp == '.' )
			{
				*pcTmp = '\0';
				break;
			}
		}

		sProcName = szBuf;
		
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
#else
		signal(SIGPIPE, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		//	signal(SIGTERM, sig_term);
		signal(SIGUSR1, SIG_IGN);
		signal(SIGUSR2, SIG_IGN);
		signal(SIGALRM, SIG_IGN);

		signal(SIGCHLD,SIG_IGN);
#endif

		if (0 == theMgr.Init(sProcName))
		{
			theMgr.Start();
			theMgr.Run();
			theMgr.Stop();
			theMgr.Finish();

			
			//退出时未作所有线程同步，简单sleep
			msleep(1);
		}
#ifdef WIN32
		WSACleanup();
#endif
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


