#pragma once


#include <7zip/IStream.h>
#include <Common/MyCom.h>
#include "ProgressCallback.h"


namespace SevenZippp
{
namespace intl
{
	class OutStreamWrapper : public IOutStream
	{
	private:

		long				m_refCount;
		CMyComPtr< IOutStream >	m_baseStream;
		CProgressCallback* m_callback;

	public:

		OutStreamWrapper( const CMyComPtr< IOutStream >& baseStream, CProgressCallback *callback = 0 );
		virtual ~OutStreamWrapper();

		STDMETHOD(QueryInterface)( REFIID iid, void** ppvObject );
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// ISequentialOutStream
		STDMETHOD(Write)( const void* data, UInt32 size, UInt32* processedSize );

		// IOutStream
		STDMETHOD(Seek)( Int64 offset, UInt32 seekOrigin, UInt64* newPosition );
		STDMETHOD(SetSize)( UInt64 newSize );
	};
}
}
