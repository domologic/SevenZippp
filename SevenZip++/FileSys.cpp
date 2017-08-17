#include "stdafx.h"
#include "FileSys.h"

#include <algorithm>
#ifdef __linux__

#define _T(s) s
#include <dirent.h>
#include <Common/Common.h>
#define MAX_PATH 0x00000104 
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "FileStream.h"
#endif
//#include "PathScanner.h"
//#include <ShlObj.h>


namespace SevenZippp
{
namespace intl
{

/*class ScannerCallback : public PathScanner::Callback
{
private:

	std::vector< FilePathInfo > m_files;
	bool m_recursive;
	bool m_onlyFirst;

public:

	ScannerCallback( bool recursive, bool onlyFirst = false ) : m_recursive( recursive ), m_onlyFirst( onlyFirst ) {}
	const std::vector< FilePathInfo >& GetFiles() { return m_files; }

	virtual bool ShouldDescend( const FilePathInfo& directory ) { return m_recursive; }
	virtual void ExamineFile( const FilePathInfo& file, bool& exit )
	{
		m_files.push_back( file );
		if ( m_onlyFirst )
		{
			exit = true;
		}
	}
};*/


class IsEmptyCallback /*: public PathScanner::Callback*/
{
private:

	bool m_isEmpty;

public:

	IsEmptyCallback() : m_isEmpty( true ) {}
	bool IsEmpty() const { return m_isEmpty; }

	virtual bool ShouldDescend( const FilePathInfo& directory ) { return true; }
	virtual void ExamineFile( const FilePathInfo& file, bool& exit ) { m_isEmpty = false; exit = true; }
};


TString FileSys::GetPath( const TString& filePath )
{
	// Find the last "\" or "/" in the string and return it and everything before it.
	TString path = filePath;
	std::replace(path.begin(), path.end(), '\\', '/');
	signed int index  = path.rfind( _T( '/' ) ); // it needs to be signed so a -1 is smaller than a small positiv index.
	if ( (unsigned) index == std::string::npos )
	{
		// No path sep.
		return TString();
	}
	else if ( (unsigned ) index + 1 >= path.size() )
	{
		// Last char is path sep, the whole thing is a path.
		return path;
	}
	else
	{
		return path.substr( 0, index + 1 );
	}
}

TString FileSys::GetFileName( const TString& filePathOrName )
{
	// Find the last "\" or "/" in the string and return everything after it.
	TString PathOrName = filePathOrName;
	std::replace(PathOrName.begin(), PathOrName.end(), '\\', '/');
	size_t index = PathOrName.rfind( _T( '/' ) );

	if ( index == std::string::npos )
	{
		// No path sep, return the whole thing.
		return PathOrName;
	}
	else if ( index + 1 >= PathOrName.size() )
	{
		// Last char is path sep, no filename.
		return TString();
	}
	else
	{
		return PathOrName.substr( index + 1, PathOrName.size() - ( index + 1 ) );
	}
}

TString FileSys::AppendPath( const TString& left, const TString& right )
{
	if ( left.empty() )
	{
		return right;
	}

	TCHAR lastChar = left[ left.size() - 1 ];
	if ( lastChar == _T( '\\' ) || lastChar == _T( '/' ) )
	{
		return left + right;
	}
	else
	{
		return left + _T( "/" ) + right;
	}
}

TString FileSys::ExtractRelativePath( const TString& basePath, const TString& fullPath )
{
	if ( basePath.size() >= fullPath.size() )
	{
		return TString();
	}

	if ( basePath != fullPath.substr( 0, basePath.size() ) )
	{
		return TString();
	}

	return fullPath.substr( basePath.size(), fullPath.size() - basePath.size() );
}

bool FileSys::DirectoryExists( const TString& path )
{
#ifdef _WIN32
	DWORD attributes = GetFileAttributes( path.c_str() );

	if ( attributes == INVALID_FILE_ATTRIBUTES )
	{
		return false;
	}
	else
	{
		return ( attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
	}
#else
	if ( path.length() == 0) return false;

	DIR *pDir;
	bool bExists = false;

	pDir = opendir (path.c_str());

	if (pDir != NULL)
	{
		bExists = true;    
		(void) closedir (pDir);
	}

    return bExists;
#endif
}

bool FileSys::IsDirectoryEmptyRecursive( const TString& path )
{
#ifdef _WIN32
	IsEmptyCallback cb;
	PathScanner::Scan( path, cb );
	return cb.IsEmpty();
#else
	struct dirent *d;
	DIR *dir = opendir(path.c_str());
	if (dir == NULL) //Not a directory or doesn't exist
		return true;
	while ((d = readdir(dir)) != NULL) {
		std::string curName = std::string(d->d_name);
		if (curName.compare(".") == 0 || curName.compare("..") == 0)
			continue;
		if (path.at(path.size() - 1) != '/')
			curName = "/" + curName;
		curName = path + curName;
		struct stat s;
		if (stat(curName.c_str(), &s) == -1) {
			cout << "Error with stat!! Filename: " << curName.c_str() << endl;
			perror("stat");
		}
		if( S_ISDIR(s.st_mode)) //it's a directory
		{
			
			if (!IsDirectoryEmptyRecursive(curName)) {
				closedir(dir);
				return false;
			}
		} else {
			closedir(dir);
			return false;
		}
		/*else if( S_ISREG(s.st_mode)) //it's a file
		{
			closedir(dir);
			return false;
		} else if (s.st_mode & S_IFMT) {
			printf("S_IFMT: %s\n", curName.c_str());
			closedir(dir);
			return false;
		} else if (s.st_mode & S_IFBLK) {
			printf("S_IFBLK: %s\n", curName.c_str());
		} else if (s.st_mode & S_IFCHR) {
			printf("S_IFCHR: %s\n", curName.c_str());
		} else if (s.st_mode & S_IFLNK) {
			printf("S_IFLNK: %s\n", curName.c_str());
		} else {
			printf("Found something, that's neither a file, nor a directory, called: %s\n", curName.c_str());
		}*/
		
	}
	closedir(dir);
	return true;
#endif
}

bool FileSys::CreateDirectoryTree( const TString& path )
{
#ifdef _WIN32
	int ret = SHCreateDirectoryEx( NULL, path.c_str(), NULL );
	return ret == ERROR_SUCCESS;
#else
	std::string s = path.c_str(); /* Code found here: https://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux */
    size_t pre=0,pos;
    std::string dir;
    int mdret;

    if (s[s.size()-1]!='/'){
        // force trailing / so we can handle everything in loop
        s+='/';
    }

    while ((pos=s.find_first_of('/',pre))!=std::string::npos) {
        dir=s.substr(0,pos++);
        pre=pos;
        if(dir.size()==0) continue; // if leading / first time is 0 length
        if((mdret=mkdir(dir.c_str(), 0755)) && errno!=EEXIST) {
            return mdret;
        }
    }
    return mdret;

#endif
}

std::vector< FilePathInfo > FileSys::GetFile( const TString& filePathOrName )
{
	TString path = FileSys::GetPath( filePathOrName );
	TString name = FileSys::GetFileName( filePathOrName );
	
	// copy file info
	FilePathInfo pathinfo;
	pathinfo.FileName = name;
	/*pathinfo.FilePath = path;
	if (pathinfo.FilePath.size() != 0 && pathinfo.FilePath.at(pathinfo.FilePath.size() - 1) != '/')
		pathinfo.FilePath += "/"; 
	pathinfo.FilePath += name; */
	pathinfo.FilePath = name;
	ifstream file(name.c_str(), ios::binary | ios::ate);
	if (file.is_open()) {
		pathinfo.Size = file.tellg();
		file.close();
	} else
		pathinfo.Size = 0;
	pathinfo.IsDirectory = false;
	pathinfo.Attributes = 128; // normal file

	std::vector <FilePathInfo > out;
	out.push_back(pathinfo);
	return out;
}
#ifndef _WIN32

bool FileSys::isSpecialFilename(const TString& filename) {
	return filename.compare(".") == 0 || filename.compare("..") == 0;
}

bool FileSys::IsAllFilesPattern( const TString& searchPattern )
{
	return searchPattern == _T( "*" ) || searchPattern == _T( "*.*" );
}

bool FileSys::PatternMatch(std::string needle, std::string haystack) {
	return PatternMatch(needle.c_str(), haystack.c_str());
}

/* Found here: https://stackoverflow.com/questions/3300419/file-name-matching-with-wildcard */
bool FileSys::PatternMatch(char const *needle, char const *haystack) {
    for (; *needle!='\0'; ++needle) {
        switch (*needle) {
        case '?': ++haystack;   
                break;
        case '*': {
            size_t max = strlen(haystack);
            if (needle[1] == '\0' || max == 0)
                return true;
            for (size_t i = 0; i < max; i++)
                if (PatternMatch(needle+1, haystack + i))
                    return true;
            return false;
        }
        default:
            if (*haystack != *needle)
                return false;
            ++haystack;
        }       
    }
    return *haystack == '\0';
}
#endif

std::vector< FilePathInfo > FileSys::GetFilesInDirectory( const TString& directory, const TString& searchPattern, bool recursive )
{
#ifdef _WIN32
	ScannerCallback cb( recursive );
	PathScanner::Scan( directory, searchPattern, cb );
	return cb.GetFiles();
#else
	std::vector<FilePathInfo> paths;
	struct dirent *d;
	DIR *dir = opendir(directory.c_str());
	if (dir == NULL)  //Not a directory or doesn't exist
		return *(new std::vector< FilePathInfo >());

	while ((d = readdir(dir)) != NULL) {
		std::string curName = std::string(d->d_name);
		if (isSpecialFilename(curName))
			continue;
		if (!IsAllFilesPattern(searchPattern)) {
			if (!PatternMatch(searchPattern, curName))
				continue;
		}
		struct stat s;
		std::string fullpath = curName;
		if (directory.at(directory.size() - 1) != '/')
			fullpath = "/" + fullpath;
		fullpath = directory + fullpath;
		if (stat(fullpath.c_str(), &s) == -1) {
			cout << "Error with stat!! Filename: " << curName.c_str() << endl;
			perror("stat");
		}
		if( S_ISDIR(s.st_mode)) //it's a directory
		{
			if (recursive) {
				TString newPath = directory;
				if (newPath.at(newPath.size() - 1) != '/')
					newPath += "/";
				newPath += curName;
				std::vector<FilePathInfo> subdirs = GetFilesInDirectory(newPath, searchPattern, recursive);
				while (!subdirs.empty()) { // TODO: not an efficient way. rework this!
					paths.push_back(subdirs.back());
					subdirs.pop_back();
				}
			}
		}
		else if( S_ISREG(s.st_mode)) //it's a file
		{
			// copy file info
			FilePathInfo pathinfo;
			pathinfo.FileName = curName;
			pathinfo.FilePath = directory;
			if (pathinfo.FilePath.size() != 0 && pathinfo.FilePath.at(pathinfo.FilePath.size() - 1) != '/')
				pathinfo.FilePath += "/";
			pathinfo.FilePath += curName;
			pathinfo.Size = s.st_size;
			pathinfo.IsDirectory = false;
			pathinfo.Attributes = 128; // normal file
			
			paths.push_back(pathinfo);
			
		} /* else if (s.st_mode & S_IFMT) { // debug msgs
			printf("S_IFMT: %s\n", curName.c_str());
		} else if (s.st_mode & S_IFBLK) {
			printf("S_IFBLK: %s\n", curName.c_str());
		} else if (s.st_mode & S_IFCHR) {
			printf("S_IFCHR: %s\n", curName.c_str());
		} else if (s.st_mode & S_IFLNK) {
			printf("S_IFLNK: %s\n", curName.c_str());
		} else {
			printf("Found something, that's neither a file, nor a directory, called: %s\n", curName.c_str());
		} */
		
	}
	closedir(dir);
	return paths;
#endif
}

CMyComPtr< IInStream > FileSys::OpenFileToRead( const TString& filePath )
{
	CMyComPtr< IInStream > fileStream;
#ifdef _WIN32
	

#ifdef _UNICODE
	const WCHAR* filePathStr = filePath.c_str();
#else
	WCHAR filePathStr[MAX_PATH];

	MultiByteToWideChar( CP_UTF8, 0, filePath.c_str(), filePath.length() + 1, filePathStr, MAX_PATH );

	//mbtowc(filePathStr, filePath.c_str(), MAX_PATH);
#endif
	if ( FAILED( SHCreateStreamOnFileEx( filePathStr, 0x0L /* STGM_READ */, 0x80 /*FILE_ATTRIBUTE_NORMAL*/, FALSE, NULL, &fileStream ) ) )
	{
		return NULL;
	}
#else
	CInFileStream *fileStreamSpec = new CInFileStream;
    fileStream = fileStreamSpec;
    
    if (!fileStreamSpec->Open(filePath.c_str()))
    {
      printf("Can not open archive file %s to read\n", filePath.c_str());
      return NULL;
    }
#endif
	return fileStream;
}

CMyComPtr< IOutStream > FileSys::OpenFileToWrite( const TString& filePath )
{
	CMyComPtr< IOutStream > fileStream;
#ifdef _WIN32	
#ifdef _UNICODE
	const WCHAR* filePathStr = filePath.c_str();
#else
	WCHAR filePathStr[MAX_PATH];
	MultiByteToWideChar( CP_UTF8, 0, filePath.c_str(), filePath.length() + 1, filePathStr, MAX_PATH );
#endif /* _UNICODE */
	if ( FAILED( SHCreateStreamOnFileEx( filePathStr, STGM_CREATE | STGM_WRITE, FILE_ATTRIBUTE_NORMAL, TRUE, NULL, &fileStream ) ) )
	{
		return NULL;
	}*/
#else
	COutFileStream *fileStreamSpec = new COutFileStream;
    fileStream = fileStreamSpec;
    
    if (!fileStreamSpec->Open(filePath.c_str()))
    {
		printf("Can not open archive file %s to write\n", filePath.c_str());
		return NULL;
    }
#endif /* _WIN32 */
	return fileStream;
}

}
}

