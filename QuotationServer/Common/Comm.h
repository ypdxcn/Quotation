#ifndef _COMM_H
#define _COMM_H

// ¶ÁÐ´Ëø
class CRWLock  
{  
protected:  
	CRWLock();  
	~CRWLock();  
	void ReadLock();  
	bool TryReadLock();  
	void WriteLock();  
	bool TryWriteLock();  
	void Unlock();  

private: 
#ifndef WIN32
	pthread_rwlock_t m_rwl;
#endif
};  

// Êý¾Ý»º´æÀà
class CDataBuffer
{
private:
	char *  m_pszBuffer;   // µ±Ç°·¢ËÍºÍ½ÓÊÕµÄÊý¾Ý°ü
	ULONG   m_lBufferSize; // °ü×Ü³¤¶ÈÊý
	ULONG   m_lMaxSize;    // ×î´ó³¤¶È
	ULONG   m_lIndex;      // Ë÷Òý

	CRWLock m_rwLock; // ¶ÁÐ´Ëø

public:
	CDataBuffer();

	~CDataBuffer();

	bool Realloc(ULONG lSize); // ÔÚÒÑÓÐµÄ»ù´¡ÉÏÖØÐÂ·ÖÅä
	bool Alloc(ULONG lSize); //·ÖÅäÄÚ´æ
	void Free();   //ÊÍ·ÅÄÚ´æ
	int IsValid() { return (m_lBufferSize > 0 && m_pszBuffer != NULL); }

	int Copy(CDataBuffer* pData);
	int Copy(const char * pszBuf, ULONG lSize);
	int Add(const char * pszBuf, ULONG lSize);

	int GetBufferSize(){return m_lBufferSize;}
	const char* GetBuffer(){return m_pszBuffer;}

	CDataBuffer& operator=(const char* lpsz)
	{
		Copy(lpsz, strlen(lpsz));
		return *this;
	}

	CDataBuffer& operator=(char* lpsz)
	{
		Copy(lpsz, strlen(lpsz));
		return *this;
	}

	CDataBuffer& operator=(CDataBuffer& pData)
	{
		Copy(pData);
		return *this;
	}

};

// ¶àÏß³ÌÊý¾Ý»º´æÀà
class CMTDataBuffer : public CDataBuffer 
{
private:
	CRWLock m_rwLock; // ¶ÁÐ´Ëø

public:
	CMTDataBuffer();

	~CMTDataBuffer();

	bool Alloc(ULONG lSize); //·ÖÅäÄÚ´æ

	void Free();   //ÊÍ·ÅÄÚ´æ

	int IsValid();

	int Copy(CDataBuffer* pData);
	int Copy(const char * pszBuf, ULONG lSize);

	int GetBufferSize();
	const char* GetBuffer();
};

// ÐòÁÐ»¯»º´æÀà
class CArchiveBuffer: public CMTDataBuffer
{
public:
	CArchiveBuffer();
	~CArchiveBuffer();

	WriteToFile();
	LoadFromFile();

	// create file mapping
	void Create(const char* filename, const unsigned int size);

	// destroy file mapping
	void Destroy();

	// determines whether the file mapping is opened
	bool isOpen()
	{
		return (m_pMemory != NULL) && (m_handle != NULL);
	}

	// ï¿½Ç·ï¿½ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ú´ï¿½Ó³ï¿½ï¿½ï¿½Ä©ï¿½ï¿?
	bool isEof()
	{
		return (m_pCurrent - m_pMemory) >= m_fileSize;
	}

	// ï¿½Ç·ï¿½ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ú´ï¿½Ó³ï¿½ï¿½Ä¿ï¿½ï¿½ï¿?
	bool isBof()
	{
		return m_pCurrent <= m_pMemory;
	}

	// ï¿½Ä¼ï¿½Ö¸ï¿½ï¿½ï¿½ï¿½Ç°ï¿½Æ¶ï¿½Ò»ï¿½ï¿½size
	void forward(const unsigned int size)
	{
		m_pCurrent += size;
	}

	// ï¿½Ä¼ï¿½Ö¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Æ¶ï¿½Ò»ï¿½ï¿½size
	void backward(const unsigned int size)
	{
		m_pCurrent -= size;
	}

	// ï¿½ï¿½ï¿½ï¿½ï¿½Ä¼ï¿½Ö¸ï¿½ï¿½
	void reset()
	{
		m_pCurrent = m_pMemory;
	}

	// write data to current position of share memory
	void write(const char* data, int len);

	// read data from current position of share memory
	void read(char* data, int len);

	// properties
public:
	// ï¿½Ãµï¿½ï¿½Ú´ï¿½Ö¸ï¿½ï¿½
	char *getMemory()
	{
		return m_pMemory;
	}

	// ï¿½Ãµï¿½ï¿½ï¿½Ç°Ö¸ï¿½ï¿½
	char *getCurrent()
	{
		return m_pCurrent;
	}

	// ï¿½Ãµï¿½ï¿½ï¿½ï¿½ï¿½Ú´ï¿½Ó³ï¿½ï¿½Ä´ï¿½Ð¡
	int getSize()
	{
		return m_fileSize;
	}

private:
#ifdef WIN32
	HANDLE m_handle; // ï¿½Ä¼ï¿½Ó³ï¿½ï¿½ï¿½ï¿½
#else
	int m_handle;
#endif
	string m_strFileName; // ï¿½Ä¼ï¿½ï¿½ï¿½
	char *m_pMemory; // ï¿½Ä¼ï¿½Ó³ï¿½ï¿½ï¿½×µï¿½Ö·
	char *m_pCurrent; // ï¿½ï¿½Ç°Ö¸ï¿½ï¿½
	unsigned int m_fileSize; // ï¿½Ä¼ï¿½ï¿½ï¿½Ð¡

};

#endif