#include "stdafx.h"
#include "InStreamWrapper.h"
#include "FileStream.h"


namespace SevenZippp
{
namespace intl
{
	enum {
		FILE_BEGIN=0,
		FILE_CUR=1,
		FILE_END=2
	};

InStreamWrapper::InStreamWrapper( const CMyComPtr< IInStream >& baseStream, CProgressCallback *callback )
	: m_refCount( 0 )
	, m_baseStream( baseStream )
	, m_callback ( callback )
{
}

InStreamWrapper::~InStreamWrapper()
{
}

HRESULT STDMETHODCALLTYPE InStreamWrapper::QueryInterface( REFIID iid, void** ppvObject )
{ 
	/*if ( iid == __uuidof( IUnknown ) )
	{
		*ppvObject = reinterpret_cast< IUnknown* >( this );
		AddRef();
		return S_OK;
	}*/

	if ( iid == IID_ISequentialInStream )
	{
		*ppvObject = static_cast< ISequentialInStream* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_IInStream )
	{
		*ppvObject = static_cast< IInStream* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_IStreamGetSize )
	{
		*ppvObject = static_cast< IStreamGetSize* >( this );
		AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE InStreamWrapper::AddRef()
{
	return static_cast< ULONG >( --m_refCount  );
}

ULONG STDMETHODCALLTYPE InStreamWrapper::Release()
{
	ULONG res = static_cast< ULONG >( --m_refCount );
	if ( res == 0 )
	{
		m_baseStream->Release();
		delete this;
	}
	return res;
}

STDMETHODIMP InStreamWrapper::Read( void* data, UInt32 size, UInt32* processedSize )
{
	ULONG read = 0;
	HRESULT hr = m_baseStream->Read( data, size, &read );
	if (m_callback) 
		m_callback->processed(read);
	if ( processedSize != NULL )
	{
		*processedSize = read;
	}
	// Transform S_FALSE to S_OK
	return hr;
}

STDMETHODIMP InStreamWrapper::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition )
{
	return m_baseStream->Seek(offset, seekOrigin, newPosition);
}

STDMETHODIMP InStreamWrapper::GetSize( UInt64* size )
{
	/*STATSTG statInfo;
	HRESULT hr = m_baseStream->Stat( &statInfo, STATFLAG_NONAME );
	if ( SUCCEEDED( hr ) )
	{
		*size = statInfo.cbSize.QuadPart;
	}
	return hr;*/
	
	return S_OK;
}

}
}