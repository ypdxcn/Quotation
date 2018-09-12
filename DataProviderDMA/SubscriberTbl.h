#ifndef _SUBSCRIPB_INC_H
#define _SUBSCRIPB_INC_H
#include "Gess.h"
#include <vector>
#include "SamplerPacket.h"

using namespace std;

///结点订阅管理
class CSubscriberTbl
{

public:

	CSubscriberTbl(void);
	~CSubscriberTbl(void);
public:
	

	///注册增加用户的订阅请求 0-成功，1-失败
	int AddSubscripItem(const unsigned int & subscriberID,char & subType,vector<unsigned int>& vecInstId );

	///根据用户取消登录请求 0-成功，1-失败
	int CancelSubscriber(const unsigned int & subscriberID);

	///获取当前所有登录用户的集合
	map<unsigned int,SUB_CONTEXT> GetSubscriberMap();

	///释放成员元素
	void Finish();
private:
	map<unsigned int,SUB_CONTEXT> m_mapSubscriber;
	CGessMutex m_mutexTbl;
};
#endif