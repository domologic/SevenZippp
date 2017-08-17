#include "stdafx.h"
#include "OutStreamWrapper.h"
#include <Common/MyWindows.h>


namespace SevenZippp
{
namespace intl
{

OutStreamWrapper::OutStreamWrapper( const CMyComPtr< IOutStream >& baseStream, CProgressCallback *callback )
	: m_refCount( 0 )
	, m_baseStream( baseStream )
	, m_callback ( callback )
{
}

OutStreamWrapper::~OutStreamWrapper()
{
}

HRESULT STDMETHODCALLTYPE OutStreamWrapper::QueryInterface( REFIID iid, void** ppvObject )
{ 
//	if ( iid == __uuidof( IUnknown ) )
//	{
//		*ppvObject = static_cast< IUnknown* >( this );
//		AddRef();
//		return S_OK;
//	}

	if ( iid == IID_ISequentialOutStream )
	{
		*ppvObject = static_cast< ISequentialOutStream* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_IOutStream )
	{
		*ppvObject = static_cast< IOutStream* >( this );
		AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE OutStreamWrapper::AddRef()
{
//	return static_cast< ULONG >( InterlockedIncrement( &m_refCount ) );
	return 0;
}

ULONG STDMETHODCALLTYPE OutStreamWrapper::Release()
{
	/*ULONG res = static_cast< ULONG >( InterlockedDecrement( &m_refCount ) );
	if ( res == 0 )
	{
		delete this;
	}
	return res;*/
	return 0;
}

STDMETHODIMP OutStreamWrapper::Write( const void* data, UInt32 size, UInt32* processedSize )
{
	ULONG written = 0;
	HRESULT hr = m_baseStream->Write( data, size, &written );
	if (m_callback)
		m_callback->processed(written);

	if ( processedSize != NULL )
	{
		*processedSize = written;
	}
	return hr;
}

STDMETHODIMP OutStreamWrapper::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition )
{
	LARGE_INTEGER move;
	ULARGE_INTEGER newPos;

	move.QuadPart = offset;
	//Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) PURE;
	HRESULT hr = m_baseStream->Seek( (Int64) move.QuadPart, seekOrigin, (UInt64*) &newPos.QuadPart );
	if ( newPosition != NULL )
	{
		*newPosition =  newPos.QuadPart;
	}
	return hr;
}

STDMETHODIMP OutStreamWrapper::SetSize( UInt64 newSize )
{
	ULARGE_INTEGER size;
	size.QuadPart = newSize;
	return m_baseStream->SetSize( (UInt64) size.QuadPart );
}

}
}
