// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <OS.h>
#include <IceUtil/Unicode.h>

#ifdef __BCPLUSPLUS__
#  include <dir.h>
#  include <io.h>
#endif

using namespace std;
using namespace OS;

#ifdef _WIN32
int
OS::osstat(const string& path, structstat* buf)
{
    return ::_wstat(IceUtil::stringToWstring(path).c_str(), buf);
}

int
OS::remove(const string& path)
{
    return ::_wremove(IceUtil::stringToWstring(path).c_str());
}

int
OS::rename(const string& from, const string& to)
{
    return ::_wrename(IceUtil::stringToWstring(from).c_str(), IceUtil::stringToWstring(to).c_str());
}

int
OS::rmdir(const string& path)
{
    return ::_wrmdir(IceUtil::stringToWstring(path).c_str());
}

int
OS::mkdir(const string& path, int)
{
    return ::_wmkdir(IceUtil::stringToWstring(path).c_str());
}

FILE*
OS::fopen(const string& path, const string& mode)
{
    return ::_wfopen(IceUtil::stringToWstring(path).c_str(), IceUtil::stringToWstring(mode).c_str());
}

int
OS::open(const string& path, int flags)
{
    return ::_wopen(IceUtil::stringToWstring(path).c_str(), flags);
}

int
OS::getcwd(string& cwd)
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

int
OS::osstat(const string& path, structstat* buf)
{
    return ::stat(path.c_str(), buf);
}

int
OS::remove(const string& path)
{
    return ::remove(path.c_str());
}

int
OS::rename(const string& from, const string& to)
{
    return ::rename(from.c_str(), to.c_str());
}

int
OS::rmdir(const string& path)
{
    return ::rmdir(path.c_str());
}

int
OS::mkdir(const string& path, int perm)
{
    return ::mkdir(path.c_str(), perm);
}

FILE*
OS::fopen(const string& path, const string& mode)
{
    return ::fopen(path.c_str(), mode.c_str());
}

int
OS::open(const string& path, int flags)
{
    return ::open(path.c_str(), flags);
}

int
OS::getcwd(string& cwd)
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
