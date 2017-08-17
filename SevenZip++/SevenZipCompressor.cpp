#include "stdafx.h"
#include "SevenZipCompressor.h"
#include "CompressionFormat.h"
#include "GUIDs.h"
#include "FileSys.h"
#include "ArchiveUpdateCallback.h"
#include "OutStreamWrapper.h"
#include "PropVariant.h"
#ifdef __linux__
#include <stdio.h>
#define _T(s) s
#endif
#include "ProgressCallback.h"
#include <fstream>


namespace SevenZippp
{

using namespace intl;


const TString SearchPatternAllFiles = _T( "*" );

CMyComPtr< IOutArchive > GetArchiveWriter(const SevenZipLibrary& library, const CompressionFormatEnum& format)
{
   const GUID* guid = NULL;

   switch (format)
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

   CMyComPtr< IOutArchive > archive;
   library.CreateObject(guid, &IID_IOutArchive, reinterpret_cast< void** >(&archive));
   return archive;
}

SevenZipCompressor::SevenZipCompressor( const SevenZipLibrary& library, const TString& archivePath, const TString& password)
	: m_library( library )
	, m_archivePath( archivePath )
	, m_password ( password )
{
	m_console = library.getConsole();
	m_progressCallback = 0;
}

SevenZipCompressor::~SevenZipCompressor()
{
}

void SevenZipCompressor::SetCompressionFormat(const CompressionFormatEnum& format)
{
   m_compressionFormat = format;
}

void SevenZipCompressor::SetCompressionLevel( const CompressionLevelEnum& level )
{
	m_compressionLevel = level;
}

void SevenZipCompressor::CompressDirectory( const TString& directory, bool includeSubdirs )
{	
	FindAndCompressFiles( 
			directory, 
			SearchPatternAllFiles, 
			FileSys::GetPath( directory ), 
			includeSubdirs );
}

void SevenZipCompressor::CompressFiles( const TString& directory, const TString& searchFilter, bool includeSubdirs )
{
	FindAndCompressFiles( 
			directory, 
			searchFilter, 
			directory, 
			includeSubdirs );
}

void SevenZipCompressor::CompressAllFiles( const TString& directory, bool includeSubdirs )
{
	FindAndCompressFiles( 
			directory, 
			SearchPatternAllFiles, 
			directory, 
			includeSubdirs );
}

void SevenZipCompressor::CompressFile( const TString& filePath )
{
	std::vector< FilePathInfo > files = FileSys::GetFile( filePath );

	if ( files.empty() )
	{
		m_console->PrintMessage(string_format("File '%s' does not exist!", filePath.c_str()));
		throw SevenZipException("Error Compressing files1!");
	}

	CompressFilesToArchive( TString(), files );
}

void SevenZipCompressor::FindAndCompressFiles( const TString& directory, const TString& searchPattern, const TString& pathPrefix, bool recursion )
{
	if ( !FileSys::DirectoryExists( directory ) )
	{
		m_console->PrintMessage(string_format("Directory \"%s\" does not exist", directory.c_str()));
		throw SevenZipException("Error Compressing files2!");
	}
	
	if ( FileSys::IsDirectoryEmptyRecursive( directory ) )
	{
		m_console->PrintMessage(string_format("Directory \"%s\" is empty", directory.c_str()));
		throw SevenZipException( "Error compressing files3!" );
	}

	std::vector< FilePathInfo > files = FileSys::GetFilesInDirectory( directory, searchPattern, recursion );
	CompressFilesToArchive( pathPrefix, files );
}

void SevenZipCompressor::CompressFilesToArchive( const TString& pathPrefix, const std::vector< FilePathInfo >& filePaths )
{
   CMyComPtr< IOutArchive > archiver = GetArchiveWriter(m_library, m_compressionFormat);

	SetCompressionProperties( archiver );
	// OpenArchiveStream:
	CMyComPtr< IOutStream > fileStream = FileSys::OpenFileToWrite( m_archivePath );
	if ( fileStream == NULL )
	{
		m_console->PrintMessage(string_format("Could not create archive '%s'!", m_archivePath.c_str()));
		throw SevenZipException("Error Compressing files4!");
	}
	// return fileStream
	int FileSizeSum = 0;
	CMyComPtr< OutStreamWrapper > outFile = new OutStreamWrapper( fileStream );
	CProgressCallback cb(m_progressCallback);
	for (int i = 0; i < filePaths.size(); i++) {
		std::ifstream in(filePaths.at(i).FilePath, std::ifstream::ate | std::ifstream::binary);
		int size = in.tellg();
		FileSizeSum += size;
	}
	cb.setSize(FileSizeSum);
	CMyComPtr< ArchiveUpdateCallback > callback = new ArchiveUpdateCallback( pathPrefix, filePaths, m_password, m_console, &cb );

	HRESULT hr = archiver->UpdateItems( outFile, (UInt32) filePaths.size(), callback );
	
	if ( hr != S_OK ) // returning S_FALSE also indicates error
	{
		m_console->PrintMessage(string_format("%s: HRESULT = 0x%08X\n", "Compress Archive", hr));
		throw SevenZipException( "Error creating Archive!" );
	}
}

void SevenZipCompressor::SetCompressionProperties( void* outArchive )
{
	const size_t numProps = 1;
	const wchar_t* names[numProps] = { L"x" };
	CPropVariant values[numProps] = { static_cast< UInt32 >( m_compressionLevel.GetValue() ) };

	CMyComPtr< ISetProperties > setter;
	((IUnknown *) outArchive)->QueryInterface( IID_ISetProperties, reinterpret_cast< void** >( &setter ) );
	if ( setter == NULL )
	{
		throw SevenZipException( _T( "Archive does not support setting compression properties" ) );
	}

	HRESULT hr = setter->SetProperties( names, values, numProps );
	if ( hr != S_OK )
	{
		throw SevenZipException( "Error Setting compression properties" );
	}
}

}
