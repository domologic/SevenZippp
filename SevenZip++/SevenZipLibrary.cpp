#include "stdafx.h"
#include <stdio.h>
#include "SevenZipLibrary.h"
#include "PropVariant.h"
#ifdef __linux__
#include <Common/Common.h>
#include <dlfcn.h> // you need to link the libdl library. -ldl
#define _T(s) s // hotfix
#endif

typedef unsigned (* GetNumberOfFormatsFunc)(unsigned *numFormats);
typedef unsigned (* GetHandlerPropertyFunc)(PROPID propID, PROPVARIANT *value);
typedef unsigned (* GetHandlerPropertyFunc2)(unsigned index, PROPID propID, PROPVARIANT *value);


namespace SevenZippp
{
#ifdef _WIN32
const TString DefaultLibraryPath = _T( "7za.dll" );
#else
const TString DefaultLibraryPath = "./7z.so";
#endif

SevenZipLibrary::SevenZipLibrary(ConsoleCallback *console)
#ifdef _WIN32
	: m_dll( NULL )
	, m_func( NULL )
#else
	: m_library( NULL )
	, m_func( NULL )
#endif
	, m_console( console )
	, m_extsLoaded ( false )
{
}

SevenZipLibrary::~SevenZipLibrary()
{
	Free();
}

void SevenZipLibrary::GetExtensions(std::vector<std::wstring> *exts) {
	if (!m_extsLoaded) {
		if (!LoadExtensions()) {
			m_console->PrintMessage("An error occured loading the extensions!");
			return;
		}
	}
	(*exts).clear();
	for (std::vector<std::wstring>::iterator it = m_exts.begin(); it != m_exts.end(); ++it) {
		(*exts).push_back(*it);
	}
}

static void SplitString(const std::wstring &srcString, std::vector<std::wstring> &destStrings)
{
	std::wstring s;
	size_t len = srcString.length();
	if (len == 0)
		return;
	for (size_t i = 0; i < len; i++) {
		wchar_t c = srcString[i];
		if (c == L' ') {
			if (!s.empty())	{
				destStrings.push_back(s);
				s.clear();
			}
		}
		else
			s += c;
	}
	if (!s.empty())
		destStrings.push_back(s);
}

/*
 * Take a look at https://github.com/stonewell/lib7zip/blob/master/Lib7Zip/7ZipFormatInfo.cpp 
 */
bool SevenZipLibrary::LoadExtensions() {
	if (m_extsLoaded) { // nothing to do here
		return true;
	}
	if (m_library == NULL || m_func == NULL) {
		m_console->PrintMessage("Cannot load extensions, the library isn't initialized yet!");
		return false;
	}
	m_exts.clear();
	
	GetHandlerPropertyFunc getHandlerProperty = (GetHandlerPropertyFunc) (dlsym(m_library, "GetHandlerProperty"));
	GetHandlerPropertyFunc2 getHandlerProperty2 = (GetHandlerPropertyFunc2) (dlsym(m_library, "GetHandlerProperty2"));
	GetNumberOfFormatsFunc getNumberOfFormats =(GetNumberOfFormatsFunc) (dlsym(m_library, "GetNumberOfFormats"));
	
	if (getHandlerProperty == NULL && getHandlerProperty2 == NULL) {
		return false;
	}
	
	unsigned numFormats = 1;
	if (getNumberOfFormats != NULL) {
		if (getNumberOfFormats(&numFormats) != S_OK)
			return false;
	}
	
	if (getHandlerProperty2 == NULL)
		numFormats = 1;
		
	for (unsigned i = 0; i < numFormats; i++) {
		intl::CPropVariant prop;
		std::wstring ext;
		PROPID propId = 2; // kExtension
		HRESULT hr;
		if (getHandlerProperty2 != NULL) 
			hr = getHandlerProperty2(i, propId, &prop);
		else
			hr = getHandlerProperty(propId, &prop);
		if (hr != S_OK) {
			m_console->PrintMessage("Error getting properties! Cannot load extensions!");
			return false;
		}
		if (prop.vt == VT_BSTR) {
			ext = prop.bstrVal;
		} else if (prop.vt != VT_EMPTY) {
			continue;
		}
		std::string localext(ext.begin(), ext.end());
		SplitString(ext, m_exts);
	}
	m_extsLoaded = true;
	return true;
}

void SevenZipLibrary::Load()
{
	Load( DefaultLibraryPath );
}

void SevenZipLibrary::Load( const TString& libraryPath )
{
#ifdef _WIN32
	Free();
	m_dll = LoadLibrary( libraryPath.c_str() );
	if ( m_dll == NULL )
	{
		throw SevenZipException( GetWinErrMsg( _T( "LoadLibrary" ), GetLastError() ) );
	}

	m_func = reinterpret_cast< CreateObjectFunc >( GetProcAddress( m_dll, "CreateObject" ) );
	if ( m_func == NULL )
	{
		Free();
		throw SevenZipException( _T( "Loaded library is missing required CreateObject function" ) );
	}
#else
	Free();
	m_library = dlopen(libraryPath.c_str(), RTLD_LOCAL | RTLD_NOW);
	if ( m_library == NULL)
		throw SevenZipException(dlerror());
	//*(void **)(&m_func) = dlsym(m_library, "CreateObject");
	m_func = ( CreateObjectFunc ) (dlsym(m_library, "CreateObject"));
	//m_func = dlsym(m_library, "CreateObject");
	if (m_func == NULL)
		throw SevenZipException(dlerror());
#endif
}

void SevenZipLibrary::Free()
{
#ifdef _WIN32
	if ( m_dll != NULL )
	{
		FreeLibrary( m_dll );
		m_dll = NULL;
		m_func = NULL;
	}
#else
	if (m_library != NULL) {
		if(dlclose(m_library) != 0) {
			TString errstr("An error occured closing the library: ");
			errstr.append(dlerror());
			throw SevenZipException(errstr);
		}
		m_library = NULL;
		m_func = NULL;
	}
#endif
}

void SevenZipLibrary::CreateObject( const void* clsID, const void* interfaceID, void** outObject ) const
{
	if ( m_func == NULL )
	{
		throw SevenZipException( _T( "Library is not loaded" ) );
	}
#ifdef _WIN32
	HRESULT hr = m_func( clsID, interfaceID, outObject );
	if ( FAILED( hr ) )
	{
		throw SevenZipException( GetCOMErrMsg( _T( "CreateObject" ), hr ) );
	}
#else
	/*if(m_func(&clsID, &interfaceID, outObject) != H_OK)
		throw SevenZipException( _T( "Cannot get CreateObject" ) );*/
	HRESULT hr = m_func(clsID, interfaceID, outObject);
	if( hr != 0x0 )
		m_console->PrintMessage(string_format("Error in %s: HRESULT = 0x%08X", "CreateObject", hr));
	
#endif
}

}
