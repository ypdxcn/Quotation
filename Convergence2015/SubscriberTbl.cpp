#include "SubscriberTbl.h"

CSubscriberTbl::CSubscriberTbl(void)
{

}

CSubscriberTbl::~CSubscriberTbl(void)
{
	Finish();
}
///注册增加用户的订阅请求 0-成功，1-失败
int CSubscriberTbl::AddSubscripItem(const unsigned int & subscriberID,char & subType,vector<unsigned int>& vecInstId )
{
	try
	{
		CGessGuard guard(m_mutexTbl);

		SUB_CONTEXT subContext;
		//memset(subContext,0,sizeof(SUB_CONTEXT));

		subContext.nodeId=subscriberID;
		subContext.subType = subType;
		subContext.vecInstId = vecInstId;

		m_mapSubscriber[subscriberID] = subContext;

		/*map<unsigned int,SUB_CONTEXT> ::iterator it=m_mapSubscriber.find(subscriberID);
		if (it != m_mapSubscriber.end())
		{
			m_mapSubscriber[subscriberID] = subContext;
		}
		else
		{
			m_mapSubscriber[subscriberID] = subContext;
		}*/
		return 0;
	}
	catch (...)
	{
		return -1;
	}	
}



///根据用户取消登录请求 0-成功，1-失败
int CSubscriberTbl::CancelSubscriber(const unsigned int & subscriberID )
{
	try
	{	
		CGessGuard guard(m_mutexTbl);

		map<unsigned int,SUB_CONTEXT> ::iterator it=m_mapSubscriber.find(subscriberID);
		if (it != m_mapSubscriber.end())
		{
			m_mapSubscriber.erase(it);
		}
		
		return 1;
	}
	catch (...)
	{
		return -1;
	}
	
}

 /***
   *  获取当前所有登录用户的集合
   * author: yfy   2010/08/20  
   * param: 
   * 
   * return 
   */
map<unsigned int,SUB_CONTEXT> CSubscriberTbl::GetSubscriberMap()
{
	CGessGuard guard(m_mutexTbl);

	return m_mapSubscriber;
}

///释放成员变量元素
void CSubscriberTbl::Finish()
{
	m_mapSubscriber.clear();
}