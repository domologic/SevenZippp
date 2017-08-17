// This file is based on the following file from the LZMA SDK (http://www.7-zip.org/sdk.html):
//   ./CPP/7zip/UI/Client7z/Client7z.cpp
#ifdef _WIN32
#include "StdAfx.h"
#endif
#include <stdio.h>
#include "ArchiveUpdateCallback.h"
#include "PropVariant.h"
#include "FileSys.h"
#include "7zDefines.h"
#include "InStreamWrapper.h"


namespace SevenZippp
{
namespace intl
{

ArchiveUpdateCallback::ArchiveUpdateCallback( const TString& dirPrefix, const std::vector< FilePathInfo >& filePaths, const TString& password, ConsoleCallback *console, CProgressCallback *cb )
	: m_refCount( 0 )
	, m_dirPrefix( dirPrefix )
	, m_filePaths( filePaths )
	, m_console ( console )
	, m_password ( password )
	, m_callback ( cb )
{
}

ArchiveUpdateCallback::~ArchiveUpdateCallback()
{
}

STDMETHODIMP ArchiveUpdateCallback::QueryInterface( REFIID iid, void** ppvObject )
{
	/*if ( iid == __uuidof( IUnknown ) )
	{
		*ppvObject = reinterpret_cast< IUnknown* >( this );
		AddRef();
		return S_OK;
	}*/

	if ( iid == IID_IArchiveUpdateCallback )
	{
		*ppvObject = static_cast< IArchiveUpdateCallback* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword2 )
	{
		*ppvObject = static_cast< ICryptoGetTextPassword2* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_ICompressProgressInfo )
	{
		*ppvObject = static_cast< ICompressProgressInfo* >( this );
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ArchiveUpdateCallback::AddRef()
{
	/*return static_cast< ULONG >( InterlockedIncrement( &m_refCount ) );*/
	return 0;
}

STDMETHODIMP_(ULONG) ArchiveUpdateCallback::Release()
{
	/*ULONG res = static_cast< ULONG >( InterlockedDecrement( &m_refCount ) );
	if ( res == 0 )
	{
		delete this;
	}
	return res;*/
	return 0;
}

STDMETHODIMP ArchiveUpdateCallback::SetTotal( UInt64 size )
{
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::SetCompleted( const UInt64* completeValue )
{
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::GetUpdateItemInfo( UInt32 index, Int32* newData, Int32* newProperties, UInt32* indexInArchive )
{
	// Setting info for Create mode (vs. Append mode).
	// TODO: support append mode
	if ( newData != NULL )
	{
		*newData = 1;
	}

	if ( newProperties != NULL )
	{
		*newProperties = 1;
	}

	if ( indexInArchive != NULL )
	{
		*indexInArchive = static_cast< UInt32 >( -1 ); // TODO: UInt32.Max
	}

	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::GetProperty( UInt32 index, PROPID propID, PROPVARIANT* value )
{
	CPropVariant prop;

	if ( propID == kpidIsAnti )
	{
		prop = false;
		prop.Detach( value );
		return S_OK;
	}

	if ( index >= m_filePaths.size() )
	{
		return E_INVALIDARG;
	}
	const FilePathInfo& fileInfo = m_filePaths.at( index );
	switch ( propID )
	{
		case kpidPath:		prop = FileSys::ExtractRelativePath( m_dirPrefix, fileInfo.FilePath ).c_str(); prop.Detach( value ); break;
		case kpidIsDir:		prop = fileInfo.IsDirectory; prop.Detach( value ); break;
		case kpidSize:		prop = fileInfo.Size; prop.Detach( value ); break;
		case kpidAttrib:	prop = fileInfo.Attributes; prop.Detach( value ); break;
#ifdef _WIN32
		case kpidCTime:		prop = fileInfo.CreationTime; prop.Detach( value ); break;
		case kpidATime:		prop = fileInfo.LastAccessTime; prop.Detach( value ); break;
		case kpidMTime:		prop = fileInfo.LastWriteTime; prop.Detach( value ); break;
#endif
	}
		
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::GetStream( UInt32 index, ISequentialInStream** inStream )
{
	if ( index >= m_filePaths.size() )
	{
		return E_INVALIDARG;
	}

	const FilePathInfo& fileInfo = m_filePaths.at( index );
	if ( fileInfo.IsDirectory )
	{
		return S_OK;
	}

	CMyComPtr< IInStream > fileStream = FileSys::OpenFileToRead( fileInfo.FilePath );
	if ( fileStream == NULL )
	{
#ifdef _WIN32
		return HRESULT_FROM_WIN32( GetLastError() );
#else
		m_console->PrintMessage(string_format("Error creating Filestream, cannot open file: %s!", fileInfo.FilePath.c_str()));
		return S_FALSE;
#endif
	}

	CMyComPtr< InStreamWrapper > wrapperStream = new InStreamWrapper( fileStream, m_callback );
	*inStream = wrapperStream.Detach();

	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::SetOperationResult( Int32 operationResult )
{
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::CryptoGetTextPassword2( Int32* passwordIsDefined, BSTR* password )
{
	if (m_password.size() == 0) {
		/*m_console->PrintMessage("Password is not defined!");
		return E_ABORT;*/
		*passwordIsDefined = (Int32) false;
	} else {
		*passwordIsDefined = (Int32) true;
	}
	std::wstring wpassword(m_password.begin(), m_password.end());
	*password = AllocateBSTR(wpassword.c_str());
	return S_OK;
}

STDMETHODIMP ArchiveUpdateCallback::SetRatioInfo( const UInt64* inSize, const UInt64* outSize )
{
	return S_OK;
}

}
}
