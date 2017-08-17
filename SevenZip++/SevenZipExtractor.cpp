#include "stdafx.h"
#include "SevenZipExtractor.h"
#include "GUIDs.h"
#include "FileSys.h"
#include "CompressionFormat.h"
#include "ArchiveOpenCallback.h"
#include "ArchiveExtractCallback.h"
#include "InStreamWrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "PropVariant.h"
#include "ProgressCallback.h"


#ifdef __linux__
#define _T(s) s
#include "7zip/Common/FileStreams.h"
#endif

namespace SevenZippp
{

using namespace intl;


CMyComPtr< IInArchive > GetArchiveReader( const SevenZipLibrary& library, const CompressionFormatEnum& format)
{
	const GUID* guid = NULL;

	switch ( format )
	{
	case CompressionFormat::Zip:
		guid = &CLSID_CFormatZip;
		break;

	case CompressionFormat::GZip:
		guid = &CLSID_CFormatGZip;
		break;

	case CompressionFormat::BZip2:
		guid = &CLSID_CFormatBZip2;
		break;

	case CompressionFormat::Rar:
		guid = &CLSID_CFormatRar;
		break;

	case CompressionFormat::Tar:
		guid = &CLSID_CFormatTar;
		break;

	case CompressionFormat::Iso:
		guid = &CLSID_CFormatIso;
		break;

	case CompressionFormat::Cab:
		guid = &CLSID_CFormatCab;
		break;

	case CompressionFormat::Lzma:
		guid = &CLSID_CFormatLzma;
		break;

	case CompressionFormat::Lzma86:
		guid = &CLSID_CFormatLzma86;
		break;

	default:
		guid = &CLSID_CFormat7z;
		break;
	}
	
	CMyComPtr< IInArchive > archive;
	library.CreateObject( guid, &IID_IInArchive, (void**)&archive);
	return archive;
}


SevenZipExtractor::SevenZipExtractor( const SevenZipLibrary& library, const TString& archivePath )
	: m_library( library )
	, m_archivePath( archivePath )
//	, m_format( CompressionFormat::SevenZip )
{
	m_console = library.getConsole();
	m_useOwnStream = true;
	m_progressCallback = 0;
}

SevenZipExtractor::SevenZipExtractor( const SevenZipLibrary& library, C7ZipInStream * archiveStream )
	: m_library( library )
//	, m_format( CompressionFormat::SevenZip )
{
	m_console = library.getConsole();
	m_archiveStream =  archiveStream;
	m_useOwnStream = false;
	m_progressCallback = 0;
}

SevenZipExtractor::~SevenZipExtractor()
{
}

void SevenZipExtractor::SetCompressionFormat( const CompressionFormatEnum& format )
{
	m_format = format;
}

bool SevenZipExtractor::isEncrypted() {
	if (m_useOwnStream) {
		// TODO: build own stream.
		return false;
	}

	CMyComPtr< IInArchive > archive = GetArchiveReader( m_library, m_format );
	CMyComPtr< ArchiveOpenCallback > openCallback = new ArchiveOpenCallback(m_console);
	CMyComPtr< InStreamWrapper > inFile = new InStreamWrapper((CMyComPtr< IInStream >)((IInStream*)m_archiveStream));
	const UInt64 scanSize = 1 << 23;
	HRESULT hr = archive->Open(inFile, &scanSize, openCallback);
	if (hr != S_OK)
		return hr == (HRESULT)0x80040001L; // Errorcode for 'E_NEEDPASSWORD'
	CPropVariant prop;
	hr = archive->GetProperty(0, kpidEncrypted, &prop);
	if (hr != S_OK)
		return (hr == (HRESULT)0x80040001L) || prop.boolVal != VARIANT_FALSE; // Errorcode for 'E_NEEDPASSWORD'
	return prop.boolVal != VARIANT_FALSE;
}


bool SevenZipExtractor::isValid() {
	if (m_useOwnStream) {
		// TODO: build own stream.
		return false;
	}
	CMyComPtr< IInArchive > archive = GetArchiveReader( m_library, m_format );
	CMyComPtr< ArchiveOpenCallback > openCallback = new ArchiveOpenCallback(m_console);
	CMyComPtr< InStreamWrapper > inFile = new InStreamWrapper((CMyComPtr< IInStream >)((IInStream*)m_archiveStream));
	const UInt64 scanSize = 1 << 23;
	HRESULT hr = archive->Open(inFile, &scanSize, openCallback);
	if (hr != S_OK)
		return false; // Errorcode for 'E_NEEDPASSWORD'
	CPropVariant prop;
	hr = archive->GetProperty(0, kpidEncrypted, &prop);
	if (hr != S_OK)
		return false; // Errorcode for 'E_NEEDPASSWORD'
	return prop.boolVal != VARIANT_FALSE;
}


void GetFullPath(int index, TString* fullPath, CMyComPtr< IInArchive > archive, SevenZipExtractor *extractor) {
	PROPVARIANT prop;
	HRESULT hr = archive->GetProperty( index, kpidPath, &prop );
	if ( hr != S_OK )
	{
		extractor->m_console->PrintMessage(string_format("Error reading properties of file"));
	}

	if ( prop.vt == VT_EMPTY )
	{
		*fullPath = "[Content]";
	}
	else if ( prop.vt != VT_BSTR )
	{
		extractor->m_console->PrintMessage(string_format("Error reading properties of file"));
	}
	else
	{
#ifdef _UNICODE
		*fullPath = prop.bstrVal;
#else
		char relPath[0x0104]; // MAX_PATH = 0x0104
#ifdef _WIN32
		int size = WideCharToMultiByte( CP_UTF8, 0, prop.bstrVal, prop.bstrVal.length(), relPath, 0x0104, NULL, NULL );
#else
		int size = wcstombs(relPath, prop.bstrVal, 0x0104);
#endif
		fullPath->assign( relPath, size );
#endif
	}
}

int GetSize(int index, CMyComPtr< IInArchive > archive, SevenZipExtractor *extractor) {
	PROPVARIANT prop;
	HRESULT hr = archive->GetProperty( index, kpidSize, &prop );
	if ( hr != S_OK )
	{
		extractor->m_console->PrintMessage(string_format("Error reading properties of file"));
	}

	switch ( prop.vt )
	{
	case VT_EMPTY:
		return 0;
	case VT_UI1: 
		return prop.bVal;
	case VT_UI2:
		return prop.uiVal;
	case VT_UI4:
		return prop.ulVal;
	case VT_UI8:
		return (UInt64)prop.uhVal.QuadPart;
	default:
		return 0;
	}
}

void SevenZipExtractor::ExtractFile( const TString& fullPath, const TString& password, C7ZipOutStream* outStream = 0 ) {
	CMyComPtr< IInArchive > archive = GetArchiveReader( m_library, m_format );
	CMyComPtr< ArchiveOpenCallback > openCallback = new ArchiveOpenCallback(m_console, password);

	CMyComPtr< InStreamWrapper > inFile;
	
	if (m_useOwnStream) {
		
		CMyComPtr< IInStream > fileStream = FileSys::OpenFileToRead( m_archivePath );
		if ( fileStream == NULL )
			throw SevenZipException("Error opening Archive!");
		inFile = new InStreamWrapper( (CMyComPtr< IInStream >)((IInStream*)fileStream) );
		
	}
	else {
		inFile = new InStreamWrapper((CMyComPtr< IInStream >)((IInStream*)m_archiveStream));
	}
	
	HRESULT hr;
	const UInt64 scanSize = 1 << 23;

	if ((hr = archive->Open(inFile, &scanSize, openCallback)) != S_OK)
	{
		m_console->PrintMessage(string_format("%s: HRESULT = 0x%08X", "Open Archive", hr));
		if (m_useOwnStream) {
			m_console->PrintMessage(string_format("ArchivePath: %s, fullpath: %s", m_archivePath.c_str(), fullPath.c_str()));
		} else {
			m_console->PrintMessage(string_format("Using custom stream!"));
		}
		throw SevenZipException("Error opening archive");
	}

	UInt32 numItems = 0;
	
	if ((hr = archive->GetNumberOfItems(&numItems))) {
		m_console->PrintMessage(string_format("%s: HRESULT = 0x%08X\n", "GetNumberOfItems", hr));
		throw SevenZipException("Error getting archive Properties");
	}
	
	TString currentPath;
	for (UInt32 i = 0; i < numItems; i++) {
		GetFullPath(i, &currentPath, archive, this);
		if (currentPath.compare(fullPath) == 0) {
			CMyComPtr< ArchiveExtractCallback > extractCallback = new ArchiveExtractCallback( archive, outStream, m_console, password, 0 );
			UInt32 indizes[1] = { i }; // it needs to be a sorted array of indizes
			hr = archive->Extract( indizes, (UInt32)1, false, extractCallback );
			if ( hr != S_OK ) // returning S_FALSE also indicates error
			{
				m_console->PrintMessage(string_format("%s: HRESULT = 0x%08X\n", "Extract Archive", hr));
				throw SevenZipException("Error extracting Archive");
			}
			return;
		}
	}
	if (m_useOwnStream)
		throw SevenZipException(string_format("Error: Cannot find file '%s' in archive '%s'!", fullPath.c_str(), m_archivePath.c_str()));
	else
		throw SevenZipException(string_format("Error: Cannot find file '%s' in given archivestream!", fullPath.c_str()));
}

void SevenZipExtractor::ExtractArchive ( const TString& destDirectory, std::string password, std::string regex ) {
	ExtractArchive(destDirectory, true, password, regex);
}

void SevenZipExtractor::ExtractArchive ( const TString& destDirectory, std::string regex) {
	ExtractArchive(destDirectory, false, "", regex);
}

static bool reg_matches(const char *str, const char *pattern)
{
    regex_t re;
    int ret;

    if (regcomp(&re, pattern, REG_EXTENDED) != 0)
        return false;

    ret = regexec(&re, str, (size_t) 0, NULL, 0);
    regfree(&re);

    if (ret == 0)
        return true;

    return false;
}

void SevenZipExtractor::ExtractArchive( const TString& destDirectory, bool passwordDefined, std::string password, std::string regex )
{
	CMyComPtr< IInArchive > archive = GetArchiveReader( m_library, m_format );
	CMyComPtr< ArchiveOpenCallback > openCallback = new ArchiveOpenCallback(m_console, "");

	CMyComPtr< InStreamWrapper > inFile;
	
	// choose stream
	if (m_useOwnStream) {
		
		CMyComPtr< IInStream > fileStream = FileSys::OpenFileToRead( m_archivePath );
		if ( fileStream == NULL )
			throw SevenZipException("Error opening Archive!");
		inFile = new InStreamWrapper( (CMyComPtr< IInStream >)((IInStream*)fileStream) );
		
	}
	else {
		inFile = new InStreamWrapper((CMyComPtr< IInStream >)((IInStream*)m_archiveStream));
	}
	
	HRESULT hr;
	const UInt64 scanSize = 1 << 23;

	if ((hr = archive->Open(inFile, &scanSize, openCallback)) != S_OK)
	{
		m_console->PrintMessage(string_format("%s: HRESULT = 0x%08X", "Open Archive", hr));
		if (m_useOwnStream) {
			m_console->PrintMessage(string_format("ArchivePath: %s, destDirectory: %s", m_archivePath.c_str(), destDirectory.c_str()));
		} else {
			m_console->PrintMessage(string_format("Using custom stream!"));
		}
		throw SevenZipException("Error opening archive");
	}

	UInt32 numItems = 0;
	
	// count objects in archive
	if ((hr = archive->GetNumberOfItems(&numItems))) {
		m_console->PrintMessage(string_format("%s: HRESULT = 0x%08X\n", "GetNumberOfItems", hr));
		throw SevenZipException("Error getting archive Properties");
	}
	
	TString currentPath;
	UInt32 indizes[numItems]; // it needs to be a sorted array of indizes
	UInt32 matchedFiles = 0;
	int fileSizeSum = 0;
	CProgressCallback cb(m_progressCallback);
	// find matching files
	for (UInt32 i = 0; i < numItems; i++) {
		GetFullPath(i, &currentPath, archive, this);
		TString filename = currentPath.substr(currentPath.find_last_of("/\\") + 1);
		if (reg_matches(filename.c_str(), regex.c_str())) {
			int fileSize = GetSize(i, archive, this);
			fileSizeSum += fileSize;
			indizes[matchedFiles++] = i;
		}
	}
	cb.setSize(fileSizeSum);
	if (matchedFiles == 0) {
		m_console->PrintMessage("Cannot find a file matching the regular expression.");
		m_console->PrintMessage(string_format("Regex: '%s'%s", regex.c_str(), m_useOwnStream? "" : string_format(", Archivename: '%s'",  m_archivePath.c_str())));
		return;
	}
	// extract matching files
	CMyComPtr< ArchiveExtractCallback > extractCallback = new ArchiveExtractCallback( archive, destDirectory, m_console, password, &cb );
	hr = archive->Extract( indizes, matchedFiles, false, extractCallback );
	if ( hr != S_OK ) // returning S_FALSE also indicates error
	{
		m_console->PrintMessage(string_format("%s: HRESULT = 0x%08X\n", "Extract Archive", hr));
		throw SevenZipException("Error extracting Archive");
	}
}

}
