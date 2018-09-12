#ifndef _QUOTATION_MIB_CONSTANT_H_
#define _QUOTATION_MIB_CONSTANT_H_

#include <string>
using namespace std;

namespace MibConst
{
	//网管MIB
	const std::string gc_sSgeLoginState			= "1.2.8.1";		//到交易所的登录状态
	const std::string gc_sSgeLoginToken			= "1.2.8.2";		//登录令牌持有状态
	const std::string gc_sSgeLoginInd			= "1.2.8.3";		//登录指示状态
	const std::string gc_sSgeLoginAlarm			= "1.2.8.4";		//登录告警状态


	
	const std::string gc_sMemQueTotal			= "1.1.5.1.1.1";
	const std::string gc_sMemQueFree			= "1.1.5.1.1.2";
	const std::string gc_sMemQueUsed			= "1.1.5.1.1.3";

	const std::string gc_sFwdCount				= "1.3.1.1.1";		//累计接收包数
	const std::string gc_sQuoPktMBytes			= "1.3.1.1.2";		//累计发送M字节数
	const std::string gc_sNowBandWidth			= "1.3.1.1.3";		//最新统计带宽
	const std::string gc_sMaxBandWidth			= "1.3.1.1.4";		//最大统计带宽
	const std::string gc_sMinBandWidth			= "1.3.1.1.5";		//最小统计带宽
	const std::string gc_sAvgBandWidth			= "1.3.1.1.6";		//平均统计带宽
	const std::string gc_sQuoPerPkt				= "1.3.1.1.7";		//每包行情数
	const std::string gc_sBytesPerPkt			= "1.3.1.1.8";		//每包字节数
	const std::string gc_sSubscribers			= "1.3.1.1.9";		//订阅数

	const std::string gc_sSamplerInPktTotal		= "1.3.2.1.1";
	const std::string gc_sSamplerFwdPktTotal	= "1.3.2.1.2";

	const std::string gc_sDelayMin				= "1.3.3.1.1";		// 
	const std::string gc_sDelayMax				= "1.3.3.1.2";		// 
	const std::string gc_sDelayAvg				= "1.3.3.1.3";		// 
	const std::string gc_sDelayLess0s			= "1.3.3.1.4";		//
	const std::string gc_sDelayLess1s			= "1.3.3.1.5";		//
	const std::string gc_sDelayLess2s			= "1.3.3.1.6";		// 
	const std::string gc_sDelayLess3s			= "1.3.3.1.7";		// 
	const std::string gc_sDelayLess5s			= "1.3.3.1.8";		// 
	const std::string gc_sDelayLess10s			= "1.3.3.1.9";		// 
	const std::string gc_sDelayLess30s			= "1.3.3.1.A";		// 
	const std::string gc_sDelayLess60s			= "1.3.3.1.B";		// 
	const std::string gc_sDelayLess120s			= "1.3.3.1.C";		// 
	const std::string gc_sDelayMore120s			= "1.3.3.1.D";		// 
	
}

#endif