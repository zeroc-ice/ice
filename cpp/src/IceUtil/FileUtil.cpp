// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/Exception.h>
#include <IceUtil/StringConverter.h>
#include <climits>
#include <string.h>

#ifdef _WIN32
#  include <process.h>
#  include <io.h>
#  ifndef ICE_OS_WINRT
#    include <Shlwapi.h>
#  endif
#else
#  include <unistd.h>
#  include <dirent.h>
#endif

using namespace std;

namespace IceUtilInternal
{
#ifdef _WIN32
const string pathsep = ";";
const string separator = "\\";
#else
const string pathsep = ":";
const string separator = "/";
#endif
}

//
// Determine if path is an absolute path
//
bool
IceUtilInternal::isAbsolutePath(const string& path)
{
    size_t i = 0;
    size_t size = path.size();

    // Skip whitespace
    while(i < size && isspace(static_cast<unsigned char>(path[i])))
    {
        ++i;
    }

#ifdef _WIN32
    // We need at least 3 non whitespace character to have
    // and absolute path
    if(i + 3 > size)
    {
        return false;
    }

    // Check for X:\ path ('\' may have been converted to '/')
    if((path[i] >= 'A' && path[i] <= 'Z') || (path[i] >= 'a' && path[i] <= 'z'))
    {
        return path[i + 1] == ':' && (path[i + 2] == '\\' || path[i + 2] == '/');
    }

    // Check for UNC path
    return (path[i] == '\\' && path[i + 1] == '\\') || path[i] == '/';
#else
    if(i >= size)
    {
        return false;
    }

    return path[i] == '/';
#endif
}

//
// Determine if a directory exists.
//
bool
IceUtilInternal::directoryExists(const string& path)
{
    IceUtilInternal::structstat st;
    if(IceUtilInternal::stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        return false;
    }
    return true;
}

//
// Determine if a directory exists and is empty.
//
#ifndef ICE_OS_WINRT
bool
IceUtilInternal::isEmptyDirectory(const string& path)
{
#   ifdef _WIN32
    return PathIsDirectoryEmptyW(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str());
#   else
    struct dirent* d;
    DIR* dir = opendir(path.c_str());
    if(dir)
    {
        bool empty = true;
        while((d = readdir(dir)))
        {
            string name(d->d_name);
            if(name != "." && name != "..")
            {
                empty = false;
                break;
            }
        }
        closedir(dir);
        return empty;
    }
    else
    {
        return false;
    }
#   endif
}
#endif

//
// Determine if a regular file exists.
//
bool
IceUtilInternal::fileExists(const string& path)
{
    IceUtilInternal::structstat st;
    if(IceUtilInternal::stat(path, &st) != 0 || !S_ISREG(st.st_mode))
    {
        return false;
    }
    return true;
}

FILE*
IceUtilInternal::freopen(const std::string& path, const std::string& mode, FILE* stream)
{
#ifdef _LARGEFILE64_SOURCE
    return freopen64(path.c_str(), mode.c_str(), stream);
#else
#  ifdef _WIN32
    //
    // Don't need to use a wide string converter, the wide strings are directly passed
    // to Windows API.
    //
    const IceUtil::StringConverterPtr converter = IceUtil::getProcessStringConverter();
    return _wfreopen(IceUtil::stringToWstring(path, converter).c_str(),
                     IceUtil::stringToWstring(mode, converter).c_str(), stream);
#  else
    return freopen(path.c_str(), mode.c_str(), stream);
#  endif
#endif
}

#ifdef _WIN32

//
// Stat
//
int
IceUtilInternal::stat(const string& path, structstat* buffer)
{
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    return _wstat(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(), buffer);
}

int
IceUtilInternal::remove(const string& path)
{
    return ::_wremove(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str());
}

int
IceUtilInternal::rename(const string& from, const string& to)
{
    //
    // Don't need to use a wide string converter, the wide strings are directly passed
    // to Windows API.
    //
    const IceUtil::StringConverterPtr converter = IceUtil::getProcessStringConverter();
    return ::_wrename(IceUtil::stringToWstring(from, converter).c_str(),
                      IceUtil::stringToWstring(to, converter).c_str());
}

int
IceUtilInternal::rmdir(const string& path)
{
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    return ::_wrmdir(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str());
}

int
IceUtilInternal::mkdir(const string& path, int)
{
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    return ::_wmkdir(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str());
}

FILE*
IceUtilInternal::fopen(const string& path, const string& mode)
{
    //
    // Don't need to use a wide string converter, the wide strings are directly passed
    // to Windows API.
    //
    const IceUtil::StringConverterPtr converter = IceUtil::getProcessStringConverter();
    return ::_wfopen(IceUtil::stringToWstring(path, converter).c_str(),
                     IceUtil::stringToWstring(mode, converter).c_str());
}

int
IceUtilInternal::open(const string& path, int flags)
{
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    if(flags & _O_CREAT)
    {
        return ::_wopen(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(),
                        flags, _S_IREAD | _S_IWRITE);
    }
    else
    {
        return ::_wopen(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(), flags);
    }
}

#ifndef ICE_OS_WINRT
int
IceUtilInternal::getcwd(string& cwd)
{
    //
    // Don't need to use a wide string converter, the wide string come
    // from Windows API.
    //
    wchar_t cwdbuf[_MAX_PATH];
    if(_wgetcwd(cwdbuf, _MAX_PATH) == NULL)
    {
        return -1;
    }
    cwd = IceUtil::wstringToString(cwdbuf, IceUtil::getProcessStringConverter());
    return 0;
}
#endif

int
IceUtilInternal::unlink(const string& path)
{
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    return _wunlink(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str());
}

int
IceUtilInternal::close(int fd)
{
#ifdef _WIN32
        return _close(fd);
#else
        return ::close(fd);
#endif
}

IceUtilInternal::FileLock::FileLock(const std::string& path) :
    _fd(INVALID_HANDLE_VALUE),
    _path(path)
{
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
#ifndef ICE_OS_WINRT
    _fd = ::CreateFileW(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(),
                        GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    CREATEFILE2_EXTENDED_PARAMETERS params;
    params.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    _fd = ::CreateFile2(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(),
                        GENERIC_WRITE, 0, OPEN_ALWAYS, &params);
#endif
    _path = path;

    if(_fd == INVALID_HANDLE_VALUE)
    {
        throw IceUtil::FileLockException(__FILE__, __LINE__, GetLastError(), _path);
    }

#ifdef __MINGW32__
    if(::LockFile(_fd, 0, 0, 0, 0) == 0)
    {
        throw IceUtil::FileLockException(__FILE__, __LINE__, GetLastError(), _path);
    }
#else
    OVERLAPPED overlaped;
    overlaped.Internal = 0;
    overlaped.InternalHigh = 0;
    overlaped.Offset = 0;
    overlaped.OffsetHigh = 0;

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
    overlaped.hEvent = nullptr;
#else
    overlaped.hEvent = 0;
#endif

    if(::LockFileEx(_fd, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 0, 0, &overlaped) == 0)
    {
        ::CloseHandle(_fd);
        throw IceUtil::FileLockException(__FILE__, __LINE__, GetLastError(), _path);
    }
#endif
    //
    // In Windows implementation we don't write the process pid to the file, as it is
    // not possible to read the file from other process while it is locked here.
    //
}

IceUtilInternal::FileLock::~FileLock()
{
    assert(_fd != INVALID_HANDLE_VALUE);
    CloseHandle(_fd);
    unlink(_path);
}

#ifndef __MINGW32__
wstring
IceUtilInternal::streamFilename(const string& filename)
{
    return stringToWstring(filename, IceUtil::getProcessStringConverter());
}
#endif

IceUtilInternal::ifstream::ifstream()
{
}

IceUtilInternal::ifstream::ifstream(const string& path, ios_base::openmode mode) :
#ifdef  __MINGW32__
    std::ifstream(path.c_str(), mode)
#else
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    std::ifstream(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(), mode)
#endif
{
}

void
IceUtilInternal::ifstream::open(const string& path, ios_base::openmode mode)
{
#ifdef  __MINGW32__
    std::ifstream::open(path.c_str(), mode);
#else
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    std::ifstream::open(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(), mode);
#endif
}

IceUtilInternal::ofstream::ofstream()
{
}

IceUtilInternal::ofstream::ofstream(const string& path, ios_base::openmode mode) :
#ifdef __MINGW32__
    std::ofstream(path.c_str(), mode)
#else
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    std::ofstream(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(), mode)
#endif
{
}

void
IceUtilInternal::ofstream::open(const string& path, ios_base::openmode mode)
{
#ifdef __MINGW32__
    std::ofstream::open(path.c_str(), mode);
#else
    //
    // Don't need to use a wide string converter, the wide string is directly passed
    // to Windows API.
    //
    std::ofstream::open(IceUtil::stringToWstring(path, IceUtil::getProcessStringConverter()).c_str(), mode);
#endif
}


#else

//
// Stat
//
int
IceUtilInternal::stat(const string& path, structstat* buffer)
{
    return ::stat(path.c_str(), buffer);
}

int
IceUtilInternal::remove(const string& path)
{
    return ::remove(path.c_str());
}

int
IceUtilInternal::rename(const string& from, const string& to)
{
    return ::rename(from.c_str(), to.c_str());
}

int
IceUtilInternal::rmdir(const string& path)
{
    return ::rmdir(path.c_str());
}

int
IceUtilInternal::mkdir(const string& path, int perm)
{
    return ::mkdir(path.c_str(), perm);
}

FILE*
IceUtilInternal::fopen(const string& path, const string& mode)
{
    return ::fopen(path.c_str(), mode.c_str());
}

int
IceUtilInternal::open(const string& path, int flags)
{
    if(flags & O_CREAT)
    {
        // By default, create with rw-rw-rw- modified by the user's umask (same as fopen).
        return ::open(path.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    }
    else
    {
        return ::open(path.c_str(), flags);
    }
}

int
IceUtilInternal::getcwd(string& cwd)
{
    char cwdbuf[PATH_MAX];
    if(::getcwd(cwdbuf, PATH_MAX) == NULL)
    {
        return -1;
    }
    cwd = cwdbuf;
    return 0;
}

int
IceUtilInternal::unlink(const string& path)
{
    return ::unlink(path.c_str());
}

int
IceUtilInternal::close(int fd)
{
    return ::close(fd);
}

IceUtilInternal::FileLock::FileLock(const std::string& path) :
    _fd(-1),
    _path(path)
{
    _fd = ::open(path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(_fd < 0)
    {
        throw IceUtil::FileLockException(__FILE__, __LINE__, errno, _path);
    }

    struct ::flock lock;
    lock.l_type = F_WRLCK; // Write lock
    lock.l_whence = SEEK_SET; // Begining of file
    lock.l_start = 0;
    lock.l_len = 0;

    //
    // F_SETLK tells fcntl to not block if it cannot
    // acquire the lock, if the lock cannot be acquired
    // it returns -1 without wait.
    //
    if(::fcntl(_fd, F_SETLK, &lock) == -1)
    {
        IceUtil::FileLockException ex(__FILE__, __LINE__, errno, _path);
        close(_fd);
        throw ex;
    }

    //
    // If there is an error after here, we close the fd,
    // to release the lock.
    //

    //
    // Now that we have acquire an excluxive write lock,
    // write the process pid there.
    //
    ostringstream os;
    os << getpid();

    if(write(_fd, os.str().c_str(), os.str().size()) == -1)
    {
        IceUtil::FileLockException ex(__FILE__, __LINE__, errno, _path);
        close(_fd);
        throw ex;
    }
}

IceUtilInternal::FileLock::~FileLock()
{
    assert(_fd > -1);
    ::close(_fd);
    unlink(_path);
}

IceUtilInternal::ifstream::ifstream()
{
}

IceUtilInternal::ifstream::ifstream(const string& path, ios_base::openmode mode) : std::ifstream(path.c_str(), mode)
{
}

void
IceUtilInternal::ifstream::open(const string& path, ios_base::openmode mode)
{
    std::ifstream::open(path.c_str(), mode);
}

IceUtilInternal::ofstream::ofstream()
{
}

IceUtilInternal::ofstream::ofstream(const string& path, ios_base::openmode mode) : std::ofstream(path.c_str(), mode)
{
}

void
IceUtilInternal::ofstream::open(const string& path, ios_base::openmode mode)
{
    std::ofstream::open(path.c_str(), mode);
}

#endif
