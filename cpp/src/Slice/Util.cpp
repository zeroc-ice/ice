// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/Util.h>
#include <IceUtil/Unicode.h>

#ifdef __hpux
#  include <unistd.h>
#endif

using namespace std;
using namespace Slice;

string
Slice::getCwd()
{
#ifdef _WIN32
    wchar_t cwdbuf[_MAX_PATH];
    if(_wgetcwd(cwdbuf, _MAX_PATH) != NULL)
    {
        return IceUtil::wstringToString(cwdbuf);
    }
#else
    char cwdbuf[PATH_MAX];
    if(::getcwd(cwdbuf, PATH_MAX) != NULL)
    {
        return cwdbuf;
    }
#endif
    return "";
}

bool
Slice::isAbsolute(const string& path)
{
#ifdef _WIN32
        if(path[0] == '\\' || path[0] == '/' || path.size() > 1 && isalpha(path[0]) && path[1] == ':')
#else
        if(path[0] == '/')
#endif
        {
            return true;
        }

        return false;
}

string
Slice::normalizePath(const string& path, bool removeDriveLetter)
{
    string result = path;
    replace(result.begin(), result.end(), '\\', '/');
    string::size_type pos;
    while((pos = result.find("//")) != string::npos)
    {
        result.replace(pos, 2, "/");
    }
    pos = 0;
    while((pos = result.find("/..", pos)) != string::npos)
    {
        string::size_type last = result.find_last_of("/", pos - 1);
        if(last != string::npos && result.substr(last, 4) != "/../")
        {
            result.erase(last, pos - last + 3);
            pos = last;
        }
        else
        {
            ++pos;
        }
    }

    if(removeDriveLetter && result.size() > 1 && isalpha(result[0]) && result[1] == ':')
    {
        result = result.substr(2);
    }

    return result;
}
