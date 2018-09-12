#ifndef _PACKET_BODY_STRUCT_IPC_OFFERING_H
#define _PACKET_BODY_STRUCT_IPC_OFFERING_H
#include <string>
#include "ArrayListMsg.h"
#include "HashtableMsg.h"

namespace ipcoffer
{
//----------------------------------------------报文接口 [StateInfo] 定义

// 结构定义:StateInfo
typedef struct tagStateInfo
{
	string       node_id              ; //源节点号
	string       node_peer_id         ; //对端节点号 请求可为空
	string		 ms_flag			  ; //主从标志
	string       ind_login            ; //登录指示
	string       login_state          ; //登录状态
	string       token				  ; //登录令牌
	string       tm_token			  ; //获得令牌的时间戳
	string		 magic_number		  ; //随机数
	string		 pkt_flag			  ; //报文请求/应答标志
}StateInfo,*PStateInfo;

}

#endif
