#if !defined(_OUTZLIB_H_)
#define _OUTZLIB_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "..\comm\systemDefine.h"

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif


#ifdef _DEBUG
	#pragma comment(lib,"..\\..\\Share\\lib\\debug\\zlib.lib") 
	//#pragma message("Automatically linking with ..\\..\\Share\\lib\\debug\\zlib.lib")  
#else
	#pragma comment(lib,"..\\..\\Share\\lib\\Release\\zlib.lib") 
	//#pragma message("Automatically linking with ..\\..\\Share\\lib\\Release\\zlib.lib")  
#endif


#include "zlib.h"

#endif // _OUTZLIB_H_
