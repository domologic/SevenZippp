#pragma once
#include "String.h"

namespace SevenZippp
{
namespace intl
{
	struct FileInfo
	{
		TString		FileName;
		#ifdef _WIN32
		FILETIME	LastWriteTime;
		FILETIME	CreationTime;
		FILETIME	LastAccessTime;
		#endif
		unsigned long long	Size;
		unsigned int		Attributes;
		bool		IsDirectory;
	};

	struct FilePathInfo : public FileInfo
	{
		TString		FilePath;
	};
}
}
