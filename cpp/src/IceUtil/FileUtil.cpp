// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/Unicode.h>
#include <climits>

#ifdef __BCPLUSPLUS__
#  include <dir.h>
#  include <io.h>
#endif
using namespace std;

//
// Detemine if path is an absolute path
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
// Detemine if a directory exists.
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

#ifdef _WIN32

//
// Stat
//
int
IceUtilInternal::stat(const string& path, structstat* buffer)
{
    return _wstat(IceUtil::stringToWstring(path).c_str(), buffer);
}

int
IceUtilInternal::remove(const string& path)
{
    return ::_wremove(IceUtil::stringToWstring(path).c_str());
}

int
IceUtilInternal::rename(const string& from, const string& to)
{
    return ::_wrename(IceUtil::stringToWstring(from).c_str(), IceUtil::stringToWstring(to).c_str());
}

int
IceUtilInternal::rmdir(const string& path)
{
    return ::_wrmdir(IceUtil::stringToWstring(path).c_str());
}

int
IceUtilInternal::mkdir(const string& path, int)
{
    return ::_wmkdir(IceUtil::stringToWstring(path).c_str());
}

FILE*
IceUtilInternal::fopen(const string& path, const string& mode)
{
    return ::_wfopen(IceUtil::stringToWstring(path).c_str(), IceUtil::stringToWstring(mode).c_str());
}

int
IceUtilInternal::open(const string& path, int flags)
{
    return ::_wopen(IceUtil::stringToWstring(path).c_str(), flags);
}

int
IceUtilInternal::getcwd(string& cwd)
{
    wchar_t cwdbuf[_MAX_PATH];
    if(_wgetcwd(cwdbuf, _MAX_PATH) == NULL)
    {
        return -1;
    }
    cwd = IceUtil::wstringToString(cwdbuf);
    return 0;
}

#else

//
// Stat
//
int
IceUtilInternal::stat(const string& path, structstat* buffer)
{
    return stat(path.c_str(), buffer);
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
    return ::open(path.c_str(), flags);
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

#endif
