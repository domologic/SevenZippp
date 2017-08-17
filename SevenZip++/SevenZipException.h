#pragma once


#include <exception>
#include "String.h"


namespace SevenZippp
{
	#ifdef _WIN32
	TString StrFmt( const TCHAR* format, ... );
	TString GetWinErrMsg( const TString& contextMessage, DWORD lastError );
	TString GetCOMErrMsg( const TString& contextMessage, HRESULT lastError );
	#else
	/*
	 * Same as string_format, strfmt etc.
	 */
	TString StrFmt( const char* format, ... );
	#endif


	class SevenZipException
	{
	protected:

		TString m_message;

	public:

		SevenZipException();
		SevenZipException( const TString& message );
		virtual ~SevenZipException();
		/*
		 * Returns the exception message, given to the constructor when thrown
		 */
		const TString& GetMessage() const;
	};
}
