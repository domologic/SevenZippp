#pragma once


#include <7zip/IStream.h>
#include <Common/MyCom.h>
#include "ProgressCallback.h"



namespace SevenZippp
{
namespace intl
{
	class InStreamWrapper : public IInStream, public IStreamGetSize
	{
	private:

		long				m_refCount;
		CMyComPtr< IInStream >	m_baseStream;
		CProgressCallback *m_callback;

	public:

		InStreamWrapper( const CMyComPtr< IInStream >& baseStream, CProgressCallback *callback = 0 );
		virtual ~InStreamWrapper();

		STDMETHOD(QueryInterface)( REFIID iid, void** ppvObject );
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// ISequentialInStream
		STDMETHOD(Read)( void* data, UInt32 size, UInt32* processedSize );

		// IInStream
		STDMETHOD(Seek)( Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

		// IStreamGetSize
		STDMETHOD(GetSize)( UInt64* size );
	};
}
}