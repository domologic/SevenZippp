#ifdef __linux__

#include <stdio.h>
#include "../../SevenZip++/7zpp.h"
#include "../../SevenZip++/ConsoleCallback.h"

class SimpleConsoleCallback : public ConsoleCallback {
public:
	virtual void PrintMessage(const char *string) {
		printf("%s\n", string);
	}
};

int PrintUsage()
{
	printf("Simple.exe [cx] ...\n");
	printf("  c <archiveName> <targetDirectory>      -- Creates an archive.\n");
	printf("  x <archiveName> <destinationDirectory> -- Extracts an archive.\n\n");
	return 0;
}

int CreateArchive(int argc, char** argv)
{
	if (argc < 4)
	{
		return PrintUsage();
	}

	const char* archiveName = argv[2];
	const char* targetDir = argv[3];
	// Note I'm lazily assuming the target is a directory rather than a file.

	SevenZippp::SevenZipLibrary lib(new SimpleConsoleCallback());
	lib.Load();

	SevenZippp::SevenZipCompressor compressor(lib, archiveName, "");
	compressor.CompressDirectory(targetDir);
	return 0;
}

int ExtractArchive(int argc, char** argv)
{
	if (argc < 4)
	{
		return PrintUsage();
	}

	const char* archiveName = argv[2];
	const char* destination = argv[3];

	SevenZippp::SevenZipLibrary lib(new SimpleConsoleCallback());
	lib.Load();
	SevenZippp::SevenZipExtractor extractor(lib, archiveName);
	extractor.ExtractArchive(destination);
	return 0;
}



int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
	if (argc < 2)
	{
		return PrintUsage();
	}

	try
	{
		switch (argv[1][0])
		{
		case 'c':
			return CreateArchive(argc, argv);
		case 'x':
			return ExtractArchive(argc, argv);
		default:
			break;
		}
	}
	catch (SevenZippp::SevenZipException& ex)
	{
		printf("Error: %s\n", ex.GetMessage().c_str());
	}

	return PrintUsage();
}

#endif