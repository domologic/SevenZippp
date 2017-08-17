// This file is based on the following file from the LZMA SDK (http://www.7-zip.org/sdk.html):
//   ./CPP/7zip/UI/Client7z/Client7z.cpp
#ifdef _WIN32
#include "StdAfx.h"
#else
/*#include <7zip/Archive/IArchive.h>*/
#endif
#include "ArchiveOpenCallback.h"
#include <stdio.h>
#include "7zDefines.h"


namespace SevenZippp
{
namespace intl
{

ArchiveOpenCallback::ArchiveOpenCallback(ConsoleCallback *console, std::string password)
	: m_refCount( 0 )
	, m_console ( console )
	, m_password ( password )
	, m_passwordDefined ( true )
{
}

ArchiveOpenCallback::ArchiveOpenCallback(ConsoleCallback *console)
	: m_refCount( 0 )
	, m_console ( console )
	, m_password ( "" )
	, m_passwordDefined ( false )
{
}

ArchiveOpenCallback::~ArchiveOpenCallback()
{
}

STDMETHODIMP ArchiveOpenCallback::QueryInterface( REFIID iid, void** ppvObject )
{
	/*if ( iid == __uuidof( IUnknown ) )
	{
		*ppvObject = reinterpret_cast< IUnknown* >( this );
		AddRef();
		return S_OK;
	}*/

	if ( iid == IID_IArchiveOpenCallback )
	{
		*ppvObject = static_cast< IArchiveOpenCallback* >( this );
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

STDMETHODIMP_(ULONG) ArchiveOpenCallback::AddRef()
{
	return static_cast< ULONG >( ++m_refCount );
}

STDMETHODIMP_(ULONG) ArchiveOpenCallback::Release()
{
	ULONG res = static_cast< ULONG >( --m_refCount );
	if ( res == 0 )
	{
		delete this;
	}
	return res;
}

STDMETHODIMP ArchiveOpenCallback::SetTotal( const UInt64* files, const UInt64* bytes )
{
	return S_OK;
}

STDMETHODIMP ArchiveOpenCallback::SetCompleted( const UInt64* files, const UInt64* bytes )
{
	return S_OK;
}

STDMETHODIMP ArchiveOpenCallback::CryptoGetTextPassword( BSTR* password )
{
	if (!m_passwordDefined) {
		m_console->PrintMessage("Password is not defined!");
		return E_ABORT;
	}
	std::wstring wpassword(m_password.begin(), m_password.end());
	*password = AllocateBSTR(wpassword.c_str());
	return S_OK;
}

}
}
