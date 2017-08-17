#ifndef SEVENZIPPP_STRING
#define SEVENZIPPP_STRING

#pragma once

#ifdef _WIN32
#include <tchar.h>
#endif
#include <string>


/*
 * For compatibility issues
 */
namespace SevenZippp
{
#ifdef _UNICODE
	typedef std::wstring TString;
#else
	typedef std::string TString;
#endif
	#ifdef __linux__
	typedef char TCHAR;
	#endif

}

#endif