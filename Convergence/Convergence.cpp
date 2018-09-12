#include <iostream>
#include "osdepend.h"
#include <string>
#include "CvgCpMgr.h"
#include <cmath>



CCvgCpMgr theMgr;



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


