
#ifndef _SAMPLER_MSG_DEF_H_
#define _SAMPLER_MSG_DEF_H_
/**
 * 消息体字段定义
 * 
 * 编号规则
 *  - 16位整数表示不同消息字段
 *  - 公共字段从0x0001开始编号，宏定义在本头文件中
 *  - 私有字段从0x1000开始编号，每个编号段包含0x0400个编号，宏定义不包含在本头文件中
 *  - 私有字段的编号段分配必须预先在本文件的末尾标注
 */

/*
 * 定长块消息协议使用的私有字段
 */
#define MSG_LOGIN_ID 		(int)0x00001000	// 登录ID
#define MSG_LOGIN_PWD		(int)0x00001001	// 登录密码
#define MSG_LOGIN_PWD_ENC	(int)0x00001002	// 密码加密方式
#define MSG_LOGIN_RESULT	(int)0x00001003	// 登录结果
#define MSG_SUBSCRIP_RESULT	(int)0x00001004	// 订阅结果

#define MSG_QUOTATION_RECS	(int)0x00001006	// 行情记录
#define MSG_SUBSCRIP_RECS   (int)0x00001007 //订阅合约列表
#define MSG_SUBSCRIP_TYPE   (int)0x00001008 //订阅类型

#define MSG_HELLO_CONTENT   (int)0x00001009 //心跳内容

// added by Jerry Lee, 2010-12-21, 增加历史数据
#define MSG_HISTORY_DATA    (int)0x00001010  //历史数据 

// added by Jerry Lee, 2011-1-17, 增加tick数据
#define MSG_TICK_DATA		(int)0x00001011  //tick数据 

// added by Jerry Lee, 2011-2-13, 增加资讯数据
#define MSG_INFO_DATA		(int)0x00001012  //资讯数据 

// added by Ben, 2011-5-29
#define MSG_BODY_NODEID		(int)0x00001013  //退出命令 

#define MSG_IFH1_DATA		(int)0x00001014  //模拟H1交易报文二进制封装
#define MSG_IFH2_DATA		(int)0x00001015  //模拟H2交易报文二进制封装
#define MSG_SV_SUB_DATA		(int)0x00001016  //订阅/退订二进制封装
#define MSG_DATE			(int)0x00001017  //
#define MSG_TIME			(int)0x00001018  //


#define YL_LOGIN				(unsigned int)0x00000001
#define YL_LOGIN_RSP			(unsigned int)0x80000001
#define YL_LOGOUT				(unsigned int)0x00000002
#define YL_LOGOUT_RSP			(unsigned int)0x80000002
#define YL_SUBSCRIP				(unsigned int)0x00000003
#define	YL_SUBSCRIP_RSP			(unsigned int)0x80000003
#define YL_UNSUBSCRIP			(unsigned int)0x00000004
#define	YL_UNSUBSCRIP_RSP		(unsigned int)0x80000004
#define YL_HELLO				(unsigned int)0x00000005
#define	YL_HELLO_RSP			(unsigned int)0x80000005

#define	YL_QUOTATION			(unsigned int)0x00000006

// added by Jerry Lee, 2010-12-21, 支持历史数据发送
#define YL_HISTORYDATA          (unsigned int)0x00000007   

// added by Jerry Lee, 2011-1-17, 支持tick数据发送
#define YL_TICKDATA             (unsigned int)0x00000008   

// added by Jerry Lee, 2011-2-13, 支持资讯数据发送
#define YL_INFODATA             (unsigned int)0x00000009   

// added by Ben, 2011-5-29, 支持资讯数据发送
#define YL_QUITMSG              (unsigned int)0x00000010 

#define YL_SV_REQ               (unsigned int)0x00000011
#define YL_SV_RSP               (unsigned int)0x80000011
#define YL_SV_NTF               (unsigned int)0x00000012
#define YL_SV_SUB_NTF           (unsigned int)0x00000013
#define YL_SV_UNSUB_NTF         (unsigned int)0x00000014
#define YL_SYNC_TIME			(unsigned int)0x00000015
#endif // _MSG_DEF_H_
