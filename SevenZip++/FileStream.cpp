#include "FileStream.h"
#include "FileSys.h"

using namespace SevenZippp;

static inline HRESULT ConvertBoolToHRESULT(bool result)
{
  #ifdef _WIN32
  if (result)
    return S_OK;
  DWORD lastError = ::GetLastError();
  if (lastError == 0)
    return E_FAIL;
  return HRESULT_FROM_WIN32(lastError);
  #else
  return result ? S_OK: E_FAIL;
  #endif
}


static const UInt32 kClusterSize = 1 << 18;
CInFileStream::CInFileStream():
  SupportHardLinks(false)
{
	
}

CInFileStream::~CInFileStream()
{
  
}

STDMETHODIMP CInFileStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
	m_file.read((char *)data, size);
	*processedSize = m_file.gcount();
	return S_OK;
}

STDMETHODIMP CInFileStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
{
	if (seekOrigin >= 3)
		return STG_E_INVALIDFUNCTION;
	int absPos = m_file.tellg();
	ios_base::seekdir origin;
	switch(seekOrigin) {
		case 0: origin = m_file.beg; break;
		case 1: origin = m_file.cur; break;
		case 2: origin = m_file.end; break;
		default: origin = m_file.cur;
	}
	m_file.seekg(offset, origin);
	if (newPosition)
		*newPosition = (UInt64) (absPos + offset);
	return S_OK;
}

STDMETHODIMP CInFileStream::GetSize(UInt64 *size)
{
	m_file.seekg(0, m_file.end);
	*size = m_file.tellg();
	m_file.seekg(0, m_file.beg); // maybe jump back to current position and not to the start?
	return S_OK;
}

// COutFileStream:

HRESULT COutFileStream::Close() {
	if (m_file.is_open())
		m_file.close();
	return S_OK;
}

STDMETHODIMP COutFileStream::Write(const void *data, UInt32 size, UInt32 *processedSize) {
	size_t before = m_file.tellp();
	m_file.write((char *)data, size);
	*processedSize = (UInt32)m_file.tellp() - before;
	m_file.flush(); // the last written item wouldn't flush automatically, because Close() isn't called, so do it manually.
	return ((size - *processedSize) == 0)?S_OK:S_FALSE;
}
STDMETHODIMP COutFileStream::Seek(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) {
	if (seekOrigin >= 3)
		return STG_E_INVALIDFUNCTION;
	int absPos = m_file.tellp();
	ios_base::seekdir origin;
	switch(seekOrigin) {
		case 0: origin = m_file.beg; break;
		case 1: origin = m_file.cur; break;
		case 2: origin = m_file.end; break;
		default: origin = m_file.cur;
	}
	m_file.seekp(offset, origin);
	if (newPosition)
		*newPosition = (UInt64) (absPos + offset);
	return S_OK;
}
STDMETHODIMP COutFileStream::SetSize(UInt64 newSize) {
	size_t curPos = m_file.tellp();
	UInt64 curSize;
	GetSize(&curSize);
	Seek(0, 0 /* begin */, 0);
	HRESULT res = Seek(newSize-curSize, 2 /* end */, 0);
	Seek(curPos, 0, 0);
	return res;
}

HRESULT COutFileStream::GetSize(UInt64 *size) {
	m_file.seekp(0, m_file.end);
	*size = m_file.tellp();
	m_file.seekp(0, m_file.beg); // maybe jump back to current position and not to the start?
	return S_OK;
}

bool COutFileStream::Open(std::string fileName) {
	m_name = fileName;
	m_file.open(fileName.c_str());
	if (!m_file.is_open()) { // maybe we cannot create/open the file, because the directory structure doesn't exist
		intl::FileSys::CreateDirectoryTree(intl::FileSys::GetPath(fileName));
		m_file.open(fileName.c_str());
	}
	return m_file.is_open();

}