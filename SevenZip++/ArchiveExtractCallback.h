// This file is based on the following file from the LZMA SDK (http://www.7-zip.org/sdk.html):
//   ./CPP/7zip/UI/Client7z/Client7z.cpp
#pragma once


#include <7zip/Archive/IArchive.h>
#include <7zip/IPassword.h>
#include <Common/MyCom.h>
#include <7zip/IStream.h>
#include <string>
#include "7zDefines.h"
#include "String.h"
#ifndef _WIN32
#include <sys/stat.h>
#endif
#include "ConsoleCallback.h"
#include "ProgressCallback.h"

namespace SevenZippp
{
namespace intl
{
	class ArchiveExtractCallback : public IArchiveExtractCallback, public ICryptoGetTextPassword
	{
	private:

		long m_refCount;
		CMyComPtr< IInArchive > m_archiveHandler;
		TString m_directory;

		TString m_relPath;
		TString m_absPath;
		bool m_isDir;
		C7ZipOutStream* m_outStream;
		
		ConsoleCallback *m_console;

		bool m_hasAttrib;
		UInt32 m_attrib;
		TString m_password;
		bool m_passwordDefined;

		bool m_hasModifiedTime;
#ifdef _WIN32
		FILETIME m_modifiedTime;
#else
		struct stat m_props;
#endif

		bool m_hasNewFileSize;
		UInt64 m_newFileSize;
		
		CProgressCallback *m_callback;

	public:

		ArchiveExtractCallback( const CMyComPtr< IInArchive >& archiveHandler, const TString& directory, ConsoleCallback *console, TString password, CProgressCallback *callback );
		ArchiveExtractCallback( const CMyComPtr< IInArchive >& archiveHandler, C7ZipOutStream* outStream, ConsoleCallback *console, TString password, CProgressCallback *callback );
		virtual ~ArchiveExtractCallback();

		STDMETHOD(QueryInterface)( REFIID iid, void** ppvObject );
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// IProgress
		STDMETHOD(SetTotal)( UInt64 size );
		STDMETHOD(SetCompleted)( const UInt64* completeValue );

		// IArchiveExtractCallback
		STDMETHOD(GetStream)( UInt32 index, ISequentialOutStream** outStream, Int32 askExtractMode );
		STDMETHOD(PrepareOperation)( Int32 askExtractMode );
		STDMETHOD(SetOperationResult)( Int32 resultEOperationResult );

		// ICryptoGetTextPassword
		STDMETHOD(CryptoGetTextPassword)( BSTR* password );

	private:

		void GetPropertyFilePath( UInt32 index );
		void GetPropertyAttributes( UInt32 index );
		void GetPropertyIsDir( UInt32 index );
		void GetPropertyModifiedTime( UInt32 index );
		void GetPropertySize( UInt32 index );
	};
}
}
