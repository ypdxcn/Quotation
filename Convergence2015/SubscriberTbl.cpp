#include "SubscriberTbl.h"

CSubscriberTbl::CSubscriberTbl(void)
{

}

CSubscriberTbl::~CSubscriberTbl(void)
{
	Finish();
}
///ע�������û��Ķ������� 0-�ɹ���1-ʧ��
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



///�����û�ȡ����¼���� 0-�ɹ���1-ʧ��
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
   *  ��ȡ��ǰ���е�¼�û��ļ���
   * author: yfy   2010/08/20  
   * param: 
   * 
   * return 
   */
map<unsigned int,SUB_CONTEXT>& CSubscriberTbl::GetSubscriberMap()
{
	CGessGuard guard(m_mutexTbl);

	return m_mapSubscriber;
}

///�ͷų�Ա����Ԫ��
void CSubscriberTbl::Finish()
{
	m_mapSubscriber.clear();
}