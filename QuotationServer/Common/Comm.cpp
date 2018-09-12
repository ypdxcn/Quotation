#include "Comm.h"

CRWLock::CRWLock() 
{
#ifndef(WIN32)
	if (pthread_rwlock_init(&m_rwl, NULL))  
		cout<<"cannot create reader/writer lock"<<endl;  
#endif
}  

CRWLock::~CRWLock()  
{  
#ifndef(WIN32)
	pthread_rwlock_destroy(&m_rwl);  
#endif
}  

void CRWLock::ReadLock()  
{  
#ifndef(WIN32)
	if (pthread_rwlock_rdlock(&m_rwl))   
		cout<<"cannot lock reader/writer lock"<<endl;  
#endif
}  

bool CRWLock::TryReadLock()  
{
#ifndef(WIN32)
	int rc = pthread_rwlock_tryrdlock(&m_rwl);  
	if (rc == 0)  
		return true;  
	else if (rc == EBUSY)  
		return false;  
	else  
		cout<<"cannot lock reader/writer lock"<<endl;  
#endif
	return false;
}  

void CRWLock::WriteLock()  
{  
#ifndef(WIN32)
	if (pthread_rwlock_wrlock(&m_rwl))   
		cout<<"cannot lock reader/writer lock"<<endl;  
#endif
}  

bool CRWLock::TryWriteLock()  
{
#ifndef(WIN32)
	int rc = pthread_rwlock_trywrlock(&m_rwl);  
	if (rc == 0)  
		return true;  
	else if (rc == EBUSY)  
		return false;  
	else  
		cout<<"cannot lock reader/writer lock"<<endl;  
#endif
	return false;  
}  

void CRWLock::UnLock()  
{  
#ifndef(WIN32)
	if (pthread_rwlock_unlock(&m_rwl))  
		cout<<"cannot unlock reader/writer lock"<<endl;  
#endif
}  

CDataBuffer::CDataBuffer()
{
	m_pszBuffer = NULL;
	m_lBufferSize = 0;
	m_lIndex = 0;
	m_lMaxSize = 0;
}

CDataBuffer::~CDataBuffer()
{
    Free();
}

bool CDataBuffer::Realloc(ULONG lSize) // 在已有的基础上重新分配
{
	if (m_lMaxSize < lSize)
	{
		if (m_pszBuffer)
		{
			char * pszOldBuffer = m_pszBuffer;
			m_pszBuffer = new char[lSize];
			if (!m_pszBuffer) return false;
			
			memcpy(m_pszBuffer, pszOldBuffer, m_lIndex);
			delete pszOldBuffer;
			m_lMaxSize = lSize;
		}
		else
		{
			m_pszBuffer = new char[lSize];
			if (!m_pszBuffer) return false;
			
			m_lMaxSize = lSize;
			m_lBufferSize = 0;
			m_lIndex = 0;			
		}
	}
	return true;
}

bool CDataBuffer::Alloc(ULONG lSize) //分配内存
{
	if (m_lMaxSize < lSize)
	{
		if (m_pszBuffer)
		{
			// 重新分配
			delete [] m_pszBuffer;
			m_pszBuffer = NULL;
			m_lBufferSize = 0;
			m_lIndex = 0;
			m_lMaxSize = 0;
		}

		m_pszBuffer = new char[lSize];

		if (m_pszBuffer)
			m_lMaxSize = lSize;
	}

	if (m_pszBuffer)
	{
		memset(m_pszBuffer, 0, lSize);
		m_lBufferSize = lSize;
		m_lIndex = 0;
	}

	return IsValid();
} 

void CDataBuffer::Free()   //释放内存
{
	if (m_pszBuffer)
	{
		// 重新分配
		delete [] m_pszBuffer;
		m_pszBuffer = NULL;
		m_lBufferSize = 0;
		m_lIndex = 0;
		m_lMaxSize = 0;
	}
}

int CDataBuffer::Copy(CDataBuffer* pData)
{
	if (pData && pData->IsValid())
	{
		if (Alloc(pData->GetBufferSize()))
		{
			memcpy(this->m_pszBuffer, pData->GetBuffer(), this->m_lBufferSize);
			m_lIndex = m_lBufferSize;
			return true;
		}
	}

	return false;
}

int CDataBuffer::Copy(const char * pszBuf, ULONG lSize)
{
	if (Alloc(lSize))
	{
		memcpy(this->m_pszBuffer, pszBuf, lSize);
		m_lIndex = m_lBufferSize;
		return true;
	}
	return false;
}

int CDataBuffer::Add(const char * pszBuf, ULONG lSize)
{
	if ((m_lMaxSize - m_lIndex) < lSize)
	{
		if (Realloc(m_lMaxSize - m_lIndex + lSize * 2))
		{
			memcpy(this->m_pszBuffer + m_lIndex, pszBuf, lSize);
			m_lIndex += lSize;
			return true;
		}
	}
}

CMTDataBuffer::CMTDataBuffer()
{

}

CMTDataBuffer::~CMTDataBuffer()
{

}

bool CMTDataBuffer::IsValid()
{
	bool bRet = false;

	m_rwLock.ReadLock();
	if (m_lBufferSize > 0 && m_pszBuffer != NULL);
	    bRet = true;
	m_rwLock.Unlock();

	return bRet;
}
bool CMTDataBuffer::Alloc(ULONG lSize) //分配内存
{
	bool bRet = false;
	m_rwLock.WriteLock();

	bRet = CDataBuffer::Alloc(lSize);
	m_rwLock.Unlock()

	return IsValid();
} 

void CMTDataBuffer::Free()   //释放内存
{
	m_rwLock.WriteLock();

	CDataBuffer::Free();

	m_rwLock.UnLock();
}

int CMTDataBuffer::Copy(CMTDataBuffer* pData)
{
	bool bRet = false;

	if (pData && pData->IsValid())
	{
		if (Alloc(pData->GetBufferSize()))
		{
			m_rwLock.WriteLock();
			memcpy(m_pszBuffer, pData->GetBuffer(), this->m_lBufferSize);
			m_rwLock.Unlock();
			bRet = true;
		}
	}

	return bRet;
}

int CMTDataBuffer::Copy(const char * pszBuf, ULONG lSize)
{
	if (Alloc(lSize))
	{
		m_rwLock.WriteLock();
		memcpy(this->m_pszBuffer, pszBuf, lSize);
		m_rwLock.Unlock();
		return true;
	}
	return false;
}
int CMTDataBuffer::GetBufferSize()
{
	m_rwLock.ReadLock();
	return m_lBufferSize;
	m_rwLock.Unlock();
}
int CMTDataBuffer::GetBuffer()
{
	m_rwLock.ReadLock();
	return m_pszBuffer;			
	m_rwLock.Unlock();
}

int CMTDataBuffer::GetIndex()
{
	m_rwLock.ReadLock();
	return m_lIndex;			
	m_rwLock.Unlock();
}