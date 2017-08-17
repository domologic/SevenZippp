#pragma once


#include "SevenZipException.h"
/*#ifdef __linux__
#include <Common/MyGuidDef.h>
#include <Common/Common.h>
#endif */
#include "ConsoleCallback.h"

namespace SevenZippp
{
	class SevenZipLibrary
	{
	private:
#ifdef _WIN32
		typedef UINT32 (WINAPI * CreateObjectFunc)( const GUID* clsID, const GUID* interfaceID, void** outObject );

		HMODULE				m_dll;
		CreateObjectFunc	m_func;
#else
		//typedef UINT32 (WINAPI * CreateObjectFunc)( const GUID* clsID, const GUID* interfaceID, void** outObject );
		typedef unsigned (* CreateObjectFunc)(const void* clsID, const void* interfaceID, void** outObject);
		void *				m_library;
		CreateObjectFunc	m_func;
#endif

		bool LoadExtensions();
		ConsoleCallback *m_console;
		bool m_extsLoaded;
		std::vector<std::wstring> m_exts;
	public:

		/*
		 * @param console The debug message callback
		 */
		SevenZipLibrary(ConsoleCallback *console);
		~SevenZipLibrary();

		/*
		 * returns the debug message callback object
		 */
		ConsoleCallback* getConsole() const {
			return m_console;
		}
		
		/*
		 * Returns all supported extentions
		 */
		void GetExtensions(std::vector<std::wstring> *exts);
		
		/*
		 * Loads the library with the default path options.
		 * This needs to be done before you can work with SevenZip++.
		 */
		void Load();
		/*
		 * Loads the library from given library path.
		 * This needs to be done before you can work with SevenZip++.
		 */
		void Load( const TString& libraryPath );
		/*
		 * Unloads the library.
		 */
		void Free();

		/*
		 * Creates an archive object from the given library specified in the "Load" method.
		 * @param clsID specifies the format
		 * @param interfaceID Most likely used for IID_IInArchive (read archive) or IID_IOutArchive (write archive)
		 * @param outObject Pointer to the object, where it's stored after initialization 
		 */
		void CreateObject( const void* clsID, const void* interfaceID, void** outObject ) const;
	};
}
