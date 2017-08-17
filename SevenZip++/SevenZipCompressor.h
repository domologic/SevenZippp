#pragma once


#include <vector>

#ifdef _WIN32
#include <atlbase.h>
#else
//#include <7zip/IStream.h>
#endif
#include "ConsoleCallback.h"
#include "SevenZipLibrary.h"
#include "FileInfo.h"
#include "CompressionFormat.h"
#include "CompressionLevel.h"
#include <functional>


namespace SevenZippp
{
	class SevenZipCompressor
	{
	private:

		const SevenZipLibrary& m_library;
		TString m_archivePath;
        CompressionFormatEnum m_compressionFormat;
		CompressionLevelEnum m_compressionLevel;
		ConsoleCallback *m_console;
		TString m_password;
		std::function<void(int step, int maxSteps)> m_progressCallback;

		
		

	public:
		/*
		 * @param library: The former initialized library
		 * @param archivePath: Destination for the compressed archive
		 * @param password: Encryption password. use "" (empty string) for no password.
		 */
		SevenZipCompressor( const SevenZipLibrary& library, const TString& archivePath, const TString& password );
		virtual ~SevenZipCompressor();

		/*
		 * Sets the compression format
		 */
		void SetCompressionFormat( const CompressionFormatEnum& format );
		/*
		 * Sets the compression level (fast, normal, etc.)
		 */
		void SetCompressionLevel( const CompressionLevelEnum& level );

		/*
		 * Compresses a whole directory and its contents (optional recursively).
		 * Note: It includes the last directory as the root in the archive, e.g. specifying "C:\Temp\MyFolder"
		 * makes "MyFolder" the single root item in archive with the files within it included.
		 */
		virtual void CompressDirectory( const TString& directory, bool includeSubdirs = true );

		/*
		 * Compresses the contents of a directory (optional recursively).
		 * Note: Excludes the last directory as the root in the archive, its contents are at root instead.
		 * E.g: specifying "C:\Temp\MyFolder" makes the files in "MyFolder" the root items in the archive.
		 * @param directory The sourcepath
		 * @param searchFilter The wildcard to filter files
		 * @param includeSubdirs Whether to compress subdirectories recursively
		 */
		virtual void CompressFiles( const TString& directory, const TString& searchFilter, bool includeSubdirs = true );
		/*
		 * Compresses all contents of a directory (optional recursively).
		 * Note: See "CompressFiles"
		 * @param directory The sourcepath
		 * @param includeSubdirs Whether to compress subdirectories recursively
		 */
		virtual void CompressAllFiles( const TString& directory, bool includeSubdirs = true );

		/*
		 * Compress just this single file as the root item in the archive.
		 */
		virtual void CompressFile( const TString& filePath );
		
		/*
		 * Sets the progress callback function for further information while compression
		 */
		void SetProgressCallback(std::function<void(int step, int maxSteps)> progressCallback) {
			m_progressCallback = progressCallback;
		}
	

	private:

		void FindAndCompressFiles( const TString& directory, const TString& searchPattern, const TString& pathPrefix, bool recursion );
		void CompressFilesToArchive( const TString& pathPrefix, const std::vector< intl::FilePathInfo >& filePaths );
		void SetCompressionProperties( void* outArchive );
	};
}
