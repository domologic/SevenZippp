#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "../../SevenZip++/7zpp.h"


int PrintUsage()
{
	_tprintf(_T("Simple.exe [cx] ...\n"));
	_tprintf(_T("  c <archiveName> <targetDirectory>      -- Creates an archive.\n"));
	_tprintf(_T("  x <archiveName> <destinationDirectory> -- Extracts an archive.\n\n"));
	return 0;
}

int CreateArchive(int argc, TCHAR** argv)
{
	if (argc < 4)
	{
		return PrintUsage();
	}

	const TCHAR* archiveName = argv[2];
	const TCHAR* targetDir = argv[3];
	// Note I'm lazily assuming the target is a directory rather than a file.

	SevenZip::SevenZipLibrary lib;
	lib.Load();

	SevenZip::SevenZipCompressor compressor(lib, archiveName);
	compressor.CompressDirectory(targetDir);
	return 0;
}

int ExtractArchive(int argc, TCHAR** argv)
{
	if (argc < 4)
	{
		return PrintUsage();
	}

	const TCHAR* archiveName = argv[2];
	const TCHAR* destination = argv[3];

	SevenZip::SevenZipLibrary lib;
	lib.Load();

	SevenZip::SevenZipExtractor extractor(lib, archiveName);
	extractor.ExtractArchive(destination);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		return PrintUsage();
	}

	try
	{
		switch (argv[1][0])
		{
		case _T('c'):
			return CreateArchive(argc, argv);
		case _T('x'):
			return ExtractArchive(argc, argv);
		default:
			break;
		}
	}
	catch (SevenZip::SevenZipException& ex)
	{
		_tprintf(_T("Error: %s\n"), ex.GetMessage().c_str());
	}

	return PrintUsage();
}
