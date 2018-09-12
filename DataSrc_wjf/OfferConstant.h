#ifndef _OFFER_CONSTANT_H_
#define _OFFER_CONSTANT_H_

#include <string>
using namespace std;

namespace OfferConst
{
	//登录结果回调值
	const int gc_nLoginCallbackOk				= 0;				//登录成功
	const int gc_nLoginCallbackFail				= 1;				//登录失败

	//登录调用结果值
	const int gc_nLoginCallOk					= 0;				//登录调用成功
	const int gc_nLoginCallFail					= -1;				//登录调用失败
	const int gc_nLoginCallRepeat				= -2;				//登录调用失败 原因重复登录
	const int gc_nLoginCallPwd					= -3;				//登录调用失败 原因密码错误

	//登出调用结果值
	const int gc_nLogoutCallOk					= 0;				//登出调用成功
	const int gc_nLogoutCallFail				= 1;				//登录调用失败

	//登出结果回调值
	const int gc_nLogoutCallbackOk				= 0;				//登出成功
	const int gc_nLogoutCallbackFail			= -1;				//登出失败

	//主从标志
	const int gc_nMaster						= 1;				//从
	const int gc_nSlave							= 0;				//主

	//登录状态
	const int gc_nStateLoginUnknown				= 0;				//未知 	
	const int gc_nStateLoginInit				= 1;				//初始化 	
	const int gc_nStateLoginPrepare				= 2;				//等待登录
	const int gc_nStateLogined					= 3;				//已经登录 
	const int gc_nStateLogouted					= 4;				//已经登出 
	const int gc_nStateLoginning				= 5;				//登录中 
	const int gc_nStateLogoutting				= 6;				//登出中 
	const int gc_nStateLoginErrRep				= 7;				//重复登录 
	const int gc_nStateLoginErrPwd				= 8;				//登录密码错误
	const int gc_nStateLoginErrOth				= 9;				//登录失败 
	const int gc_nStateLogoutErrOth				= 10;				//登出失败
	const int gc_nStateLoginClosed				= 11;				//被金交所关闭

	//登录指示 
	const int gc_nIndUnknown					= 0;				//无指示
	const int gc_nIndLogin						= 1;				//指示登录
	const int gc_nIndLogout						= 2;				//指示登出

	//与对端连接状态
	const int gc_nStateInit						= 0;				//初始未知
	const int gc_nStateConnected				= 1;				//已连接
	const int gc_nStateDisConnected				= 2;				//已中断

	//令牌状态
	const int gc_nLoginTokenHold				= 1;				//持有
	const int gc_nLoginTokenWait				= 0;				//期望持有
	const int gc_nLoginTokenGiveup				= 2;				//主动放弃

	//令牌协商结果
	const int gc_nTokenNegotiate				= 0;				//需要协商
	const int gc_nTokenWait2Hold				= 1;				//未持有协商为持有
	const int gc_nTokenHold2Wait				= 2;				//持有协商为未持有
	const int gc_nTokenWait2Wait				= 3;				//保持未持有
	const int gc_nTokenHold2Hold				= 4;				//保持持有
	
	//登录告警
	const int gc_nLoginNormal					= 0;				//登录状态正常
	const int gc_nLoginAlarm					= 1;				//登录状态告警

	const int gc_nPktRequest					= 0;				//请求
	const int gc_nPktResponse					= 1;				//应答


}

#endif