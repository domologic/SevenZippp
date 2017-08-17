// This file is based on the following file from the LZMA SDK (http://www.7-zip.org/sdk.html):
//   ./CPP/7zip/UI/Client7z/Client7z.cpp
#ifdef _WIN32
#include "StdAfx.h"
#else
#include <stdlib.h>
#include <utime.h>
#endif
#include "ArchiveExtractCallback.h"
#include "FileSys.h"
#include "OutStreamWrapper.h"
#include <stdio.h>
/*#include "PropVariant.h"


#include <comdef.h>*/

namespace SevenZippp
{
namespace intl
{

//const TString EmptyFileAlias = _T( "[Content]" );


ArchiveExtractCallback::ArchiveExtractCallback( const CMyComPtr< IInArchive >& archiveHandler, const TString& directory, ConsoleCallback *console, std::string password, CProgressCallback *callback = 0 )
	: m_refCount( 0 )
	, m_archiveHandler( archiveHandler )
	, m_directory( directory )
	, m_outStream( 0 )
	, m_isDir( false )
	, m_console( console )
	, m_password ( password )
	, m_passwordDefined ( password.size() > 0 )
	, m_callback ( callback )
{
}

ArchiveExtractCallback::ArchiveExtractCallback( const CMyComPtr< IInArchive >& archiveHandler, C7ZipOutStream* outStream, ConsoleCallback *console, std::string password, CProgressCallback *callback = 0 )
	: m_refCount( 0 )
	, m_archiveHandler( archiveHandler )
	, m_outStream( outStream )
	, m_isDir(false)
	, m_console( console )
	, m_password ( password )
	, m_passwordDefined ( password.size() > 0 )
	, m_callback ( callback )
{
}

ArchiveExtractCallback::~ArchiveExtractCallback()
{
}

STDMETHODIMP ArchiveExtractCallback::QueryInterface( REFIID iid, void** ppvObject )
{
	/*if ( iid == __uuidof( IUnknown ) )
	{
		*ppvObject = reinterpret_cast< IUnknown* >( this );
		AddRef();
		return S_OK;
	}*/

	if ( iid == IID_IArchiveExtractCallback )
	{
		*ppvObject = static_cast< IArchiveExtractCallback* >( this );
		AddRef();
		return S_OK;
	}

	if ( iid == IID_ICryptoGetTextPassword )
	{
		*ppvObject = static_cast< ICryptoGetTextPassword* >( this );
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ArchiveExtractCallback::AddRef()
{
//	return static_cast< ULONG >( InterlockedIncrement( &m_refCount ) );
	return 0;
}

STDMETHODIMP_(ULONG) ArchiveExtractCallback::Release()
{
	/*ULONG res = static_cast< ULONG >( InterlockedDecrement( &m_refCount ) );
	if ( res == 0 )
	{
		delete this;
	}
	return res;*/
	return 0;
}

STDMETHODIMP ArchiveExtractCallback::SetTotal( UInt64 size )
{
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::SetCompleted( const UInt64* completeValue )
{
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::GetStream( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode )
{
	
	// Retrieve all the various properties for the file at this index.
	GetPropertyFilePath( index );
	if ( askExtractMode != NArchive::NExtract::NAskMode::kExtract )
	{
		//return S_OK;
	}

	GetPropertyAttributes( index );
	GetPropertyIsDir( index );
	GetPropertyModifiedTime( index );
	GetPropertySize( index );
	
	// if no stream was given, create one on our own
	if (m_outStream == 0) {
		// TODO: m_directory could be a relative path
		m_absPath = FileSys::AppendPath( m_directory, m_relPath );
		

		if ( m_isDir )
		{
			// Creating the directory here supports having empty directories.
			FileSys::CreateDirectoryTree( m_absPath );
			*outStream = NULL;
			return S_OK;
		}

		
		TString absDir = FileSys::GetPath( m_absPath );
		FileSys::CreateDirectoryTree( absDir );
		
		CMyComPtr< IOutStream > fileStream = FileSys::OpenFileToWrite( m_absPath );
		if ( fileStream == NULL )
		{
			m_absPath.clear();
			return S_FALSE;
		}

		CMyComPtr< OutStreamWrapper > wrapperStream = new OutStreamWrapper( fileStream, m_callback );
		*outStream = wrapperStream.Detach();
	} else {
		// use given stream.
		*outStream = (ISequentialOutStream *) m_outStream;
	}
	
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::PrepareOperation( Int32 askExtractMode )
{
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::SetOperationResult( Int32 operationResult )
{
	if ( m_absPath.empty() )
	{
		return S_OK;
	}

	if ( m_hasModifiedTime )
	{
#ifdef _WIN32
		HANDLE fileHandle = CreateFile( m_absPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( fileHandle != INVALID_HANDLE_VALUE )
		{
			SetFileTime( fileHandle, NULL, NULL, &m_modifiedTime );
			CloseHandle( fileHandle );
		}
	}
#else
		utimbuf time;
		time.modtime = m_props.st_mtime;
		time.actime = m_props.st_atime;
		utime(m_absPath.c_str(), &time);
	}
	//chmod(m_absPath.c_str(), m_props.st_mode); // sounds good, doesn't work
#endif

	if ( m_hasAttrib )
	{
#ifdef _WIN32
		SetFileAttributes( m_absPath.c_str(), m_attrib );
#endif
	}
	
	TString fileName = FileSys::GetFileName(m_absPath);
	
	return S_OK;
}

STDMETHODIMP ArchiveExtractCallback::CryptoGetTextPassword( BSTR* password )
{
	if (!m_passwordDefined) {
		m_console->PrintMessage("Password is not defined!");
		return E_ABORT;
	}
	std::wstring wpassword(m_password.begin(), m_password.end());
	*password = AllocateBSTR(wpassword.c_str());
	return S_OK;
}

void ArchiveExtractCallback::GetPropertyFilePath( UInt32 index )
{
	PROPVARIANT prop;
	HRESULT hr = m_archiveHandler->GetProperty( index, kpidPath, &prop );
	if ( hr != S_OK )
	{
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}

	if ( prop.vt == VT_EMPTY )
	{
		//m_relPath = EmptyFileAlias;
		// static const wchar_t * const kEmptyFileAlias = L"[Content]";
		m_relPath = "[Content]";
	}
	else if ( prop.vt != VT_BSTR )
	{
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}
	else
	{
#ifdef _UNICODE
		m_relPath = prop.bstrVal;
#else
		char relPath[0x0104]; // MAX_PATH = 0x0104
#ifdef _WIN32
		int size = WideCharToMultiByte( CP_UTF8, 0, prop.bstrVal, prop.bstrVal.length(), relPath, 0x0104, NULL, NULL );
#else
		int size = wcstombs(relPath, prop.bstrVal, 0x0104);
#endif
		m_relPath.assign( relPath, size );
#endif
	}
}

void ArchiveExtractCallback::GetPropertyAttributes( UInt32 index )
{
	PROPVARIANT prop;
	HRESULT hr = m_archiveHandler->GetProperty( index, kpidAttrib, &prop );
	if ( hr != S_OK )
	{
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}

	if ( prop.vt == VT_EMPTY )
	{
		m_attrib = 0;
		m_hasAttrib = false;
	}
	else if ( prop.vt != VT_UI4 )
	{
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}
	else
	{
		m_attrib = prop.ulVal;
		m_hasAttrib = true;
	}
}

void ArchiveExtractCallback::GetPropertyIsDir( UInt32 index )
{
	PROPVARIANT prop;
	HRESULT hr = m_archiveHandler->GetProperty( index, kpidIsDir, &prop );
	if ( hr != S_OK )
	{
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}

	if ( prop.vt == VT_EMPTY )
	{
		m_isDir = false;
	}
	else if ( prop.vt != VT_BOOL )
	{
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}
	else
	{
		m_isDir = prop.boolVal != VARIANT_FALSE;
	}
}

void ArchiveExtractCallback::GetPropertyModifiedTime( UInt32 index )
{
#ifdef _WIN32
	PROPVARIANT prop;
	HRESULT hr = m_archiveHandler->GetProperty( index, kpidMTime, &prop );
	if ( hr != S_OK )
	{
		m_console->PrintMessage(string_format("Error reading properties of file");
	}

	if ( prop.vt == VT_EMPTY )
	{
		m_hasModifiedTime = false;
	}
	else if ( prop.vt != VT_FILETIME )
	{
		m_console->PrintMessage(string_format("Error reading properties of file");
	}
	else
	{
		m_modifiedTime = prop.filetime;
		m_hasModifiedTime = true;
	}
#else
	struct stat props;
	stat(m_absPath.c_str(), &props);
	m_props = props;

#endif
}

void ArchiveExtractCallback::GetPropertySize( UInt32 index )
{
	PROPVARIANT prop;
	HRESULT hr = m_archiveHandler->GetProperty( index, kpidSize, &prop );
	if ( hr != S_OK )
	{
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}

	switch ( prop.vt )
	{
	case VT_EMPTY:
		m_hasNewFileSize = false;
		return;
	case VT_UI1: 
		m_newFileSize = prop.bVal;
		break;
	case VT_UI2:
		m_newFileSize = prop.uiVal;
		break;
	case VT_UI4:
		m_newFileSize = prop.ulVal;
		break;
	case VT_UI8:
		m_newFileSize = (UInt64)prop.uhVal.QuadPart;
		break;
	default:
		m_console->PrintMessage(string_format("Error reading properties of file"));
	}

	m_hasNewFileSize = true;
}

}
}
