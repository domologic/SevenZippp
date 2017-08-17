#pragma once


#include "SevenZipLibrary.h"
/*
#include <Common/MyCom.h>
#include <7zip/IStream.h>
*/
#include "CompressionFormat.h"
#include "ConsoleCallback.h"
#include "7zDefines.h"
#include <iostream>
#include <functional>

namespace SevenZippp
{
	class SevenZipExtractor
	{
	private:

		const SevenZipLibrary& m_library;
		TString m_archivePath;
		CompressionFormatEnum m_format;
		bool m_useOwnStream;
		C7ZipInStream *m_archiveStream;
		std::function<void(int step, int maxSteps)> m_progressCallback;
		

	public:
		/*
		 * Callback object for debug and info messages
		 */
		ConsoleCallback *m_console;
		
		/*
		 * @param library The former initialized library
		 * @param archivePath Source path of an archive of which should be extracted from.
		 */
		SevenZipExtractor( const SevenZipLibrary& library, const TString& archivePath );
		/*
		 * @param library The former initialized library
		 * @param archiveStream Source stream of an archive of which should be extracted from.
		 */
		SevenZipExtractor( const SevenZipLibrary& library, C7ZipInStream * archiveStream );

		virtual ~SevenZipExtractor();

		/*
		 * Sets the compression format
		 */
		void SetCompressionFormat( const CompressionFormatEnum& format );

		/*
		 * Extracts a single file from the archive.
		 * @param fullPath The full path of the file within the archive
		 * @param password If the archive is encrypted, a password is required
		 * @param outstream A stream where the data of the file is directed to
		 */
		void ExtractFile( const TString& fullPath, const TString& password, C7ZipOutStream* outstream );
		/*
		 * Extracts an archive
		 * @param destDirectory The destination on your harddrive, where the archive should be extracted to
		 * @param regex There will only be those files extracted, which match the regular expression
		 */
		void ExtractArchive( const TString& destDirectory, std::string regex = ".*" );
		/*
		 * Extracts an archive
		 * @param destDirectory The destination on your harddrive, where the archive should be extracted to
		 * @param password If the archive is encrypted, a password is required
		 * @param regex There will only be those files extracted, which match the regular expression
		 */
		void ExtractArchive( const TString& destDirectory, std::string password, std::string regex = ".*" );
		/*
		 * !Please use other the overloaded function above!
		 * Extracts an archive
		 * @param destDirectory The destination on your harddrive, where the archive should be extracted to
		 * @param passwordDefined Whether you want to use a password to open the archive or not
		 * @param password If the archive is encrypted, a password is required
		 * @param regex There will only be those files extracted, which match the regular expression
		 */
		void ExtractArchive( const TString& destDirectory, bool passwordDefined, std::string password, std::string regex = ".*" );
		
		/*
		 * Sets a progression callback for further information of the extraction
		 */
		void SetProgressCallback(std::function<void(int step, int maxSteps)> progressCallback) {
			m_progressCallback = progressCallback;
		}
		
		/*
		 * Returns true if the archive is encrypted
		 */
		bool isEncrypted ();
		/*
		 * Returns true if the archive is valid
		 */
		bool isValid();


	};
}
