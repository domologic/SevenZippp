
#include <Common/MyCom.h>

#include <7zip/IStream.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;
namespace SevenZippp
{

class CInFileStream
:public IInStream
,public CMyUnknownImp
{
public:
	ifstream m_file;
	bool SupportHardLinks;
	

	virtual ~CInFileStream();

	CInFileStream();

	bool Open(std::string fileName)
	{
		m_file.open(fileName.c_str());
		return m_file.is_open();
	}

	bool OpenShared(std::string fileName, bool shareForWrite)
	{
		m_file.open(fileName.c_str());
		return m_file.is_open();
	}
	
	MY_QUERYINTERFACE_BEGIN2(IInStream)
	MY_QUERYINTERFACE_END
	MY_ADDREF_RELEASE
	STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);

	STDMETHOD(GetSize)(UInt64 *size);
	
};

class COutFileStream:
  public IOutStream,
  public CMyUnknownImp
{
public:

	ofstream m_file;
	std::string m_name;

	virtual ~COutFileStream() {}
	bool Create(std::string fileName, bool createAlways)
	{
		if (m_file.is_open())
			return false;
		ProcessedSize = 0;
		m_file.open(fileName.c_str());
		bool success = m_file.is_open();
		m_file.close();
		return success;
	}
	bool Open(std::string fileName);

  HRESULT Close();
  
  UInt64 ProcessedSize;


  MY_UNKNOWN_IMP1(IOutStream)

  STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
  STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition);
  STDMETHOD(SetSize)(UInt64 newSize);

  HRESULT GetSize(UInt64 *size);
};

}