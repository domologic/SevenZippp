#pragma once


#include <vector>
#include "FileInfo.h"
#ifdef __linux__
#include <Common/MyCom.h>
#include <7zip/IStream.h>
#endif
#include "GUIDs.h"



namespace SevenZippp
{
namespace intl
{
	class FileSys
	{
	public:
	
	

		static TString GetPath( const TString& filePath );
		static TString GetFileName( const TString& filePathOrName );
		static TString AppendPath( const TString& left, const TString& right );
		static TString ExtractRelativePath( const TString& basePath, const TString& fullPath );

		static bool DirectoryExists( const TString& path );
		static bool IsDirectoryEmptyRecursive( const TString& path );

		static bool CreateDirectoryTree( const TString& path );

		static std::vector< FilePathInfo > GetFile( const TString& filePathOrName );
		static std::vector< FilePathInfo > GetFilesInDirectory( const TString& directory, const TString& searchPattern, bool recursive );

#ifdef _WIN32
		static CComPtr< IStream > OpenFileToRead( const TString& filePath );
		static CComPtr< IStream > OpenFileToWrite( const TString& filePath );
#else
		static CMyComPtr< IInStream > OpenFileToRead( const TString& filePath );
		static CMyComPtr< IOutStream > OpenFileToWrite( const TString& filePath );
		static bool isSpecialFilename(const TString& filename);
		static bool IsAllFilesPattern( const TString& searchPattern );
		static bool PatternMatch(std::string needle, std::string haystack);
		static bool PatternMatch(char const *needle, char const *haystack);
#endif
	};
}
}
