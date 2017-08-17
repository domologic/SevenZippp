#pragma once

#define __int64 long long int

#include <string>
#include <cstring>

typedef std::basic_string<wchar_t> wstring;
typedef std::basic_string<char> string;

/*
 * Windows defines
 */
typedef struct {
  unsigned int Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID_CLONE;

/*
 * Copy Structure from P7ZIP library
 */
class CMyUnknownImpClone
{
public:
  unsigned long __m_RefCount;
  CMyUnknownImpClone(): __m_RefCount(0) {}

  // virtual
  ~CMyUnknownImpClone() {}
};

/*
 * Copy Structure from P7ZIP library
 */
struct IUnknownClone
{
  virtual long QueryInterface(GUID_CLONE iid, void **outObject) = 0;
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
  #ifndef _WIN32
  virtual ~IUnknownClone() {}
  #endif
};

/*
 * Copy Structure from P7ZIP library
 */
struct IInStreamClone: IUnknownClone {
	virtual int Read(void *data, unsigned int size, unsigned int *processedSize) = 0;
	virtual int Seek(long long int offset, unsigned int seekOrigin, unsigned long long int *newPosition) = 0;
	virtual int GetSize(unsigned long long int * size) = 0; // might not need this
};

/*
 * Copy Structure from P7ZIP library
 */
struct IOutStreamClone: IUnknownClone {
	virtual int Write(const void *data, unsigned int size, unsigned int *processedSize) = 0;
	virtual int Seek(long long int offset, unsigned int seekOrigin, unsigned long long int *newPosition) = 0;
	virtual int SetSize(unsigned long long int size) = 0;
};

/*
 * Copy Structure from P7ZIP library
 * Interface for InStreams with SevenZip++
 */
class C7ZipInStream: public IInStreamClone, CMyUnknownImpClone
{
public:
	long int QueryInterface(GUID_CLONE iid, void **outObject) {
		return 0;
	}
	
	unsigned long AddRef() {
		return 0;
	}
	
	unsigned long Release() {
		Seek(0, 0, 0);
		return 0;
	}
};

/*
 * Copy Structure from P7ZIP library
 * Interface for OutStreams with SevenZip++
 */
class C7ZipOutStream: public IOutStreamClone, CMyUnknownImpClone
{
public:
	long int QueryInterface(GUID_CLONE iid, void **outObject) {
		return 0;
	}
	
	unsigned long AddRef() {
		return 0;
	}
	
	unsigned long Release() {
		Seek(0, 0, 0);
		return 0;
	}
};

/*
 * Some Windows Defines
 * SevenZip++ is ported from Windows to Unix
 */
struct BSTR_data {
    short count;
    wchar_t data[];
};

typedef wchar_t *BSTR;

static inline void *AllocateForBSTR(size_t cb) { return ::malloc(cb); }

inline unsigned StringLen(const wchar_t *s)
{
  unsigned i;
  for (i = 0; s[i] != 0; i++);
  return i;
}

static BSTR AllocateBSTR(const wchar_t *sz)
{
  if (sz == 0)
    return 0;
  unsigned strLen = StringLen(sz);
  unsigned len = (strLen + 1) * sizeof(wchar_t);
  void *p = AllocateForBSTR(len + sizeof(unsigned));
  if (p == 0)
    return 0;
  *(unsigned *)p = strLen * sizeof(wchar_t); // FIXED
  void * bstr = (void *)((unsigned *)p + 1);
  memmove(bstr, sz, len); // sz does not always have "wchar_t" alignment.
  return (BSTR)bstr;
}
