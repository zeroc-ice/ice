// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/DynamicLibrary.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/StringConverter.h>

#ifndef _WIN32
#   include <dlfcn.h>
#endif

#if defined(ICE_CPP11) && defined(__GNUC__) && (__GNUC__ < 6) && defined(__GLIBCXX__)
#   define ICE_LIBSUFFIX "++11"
#endif

using namespace Ice;
using namespace IceInternal;
using namespace std;

IceUtil::Shared* IceInternal::upCast(DynamicLibrary* p) { return p; }
IceUtil::Shared* IceInternal::upCast(DynamicLibraryList* p) { return p; }

IceInternal::DynamicLibrary::DynamicLibrary() : 
    _hnd(0)
{
}

IceInternal::DynamicLibrary::~DynamicLibrary()
{
    /*
     * Closing the library here can cause a crash at program exit if
     * the application holds references to library resources in global
     * or static variables. Instead, we let the process discard the
     * library.
     *
    if(_hnd != 0)
    {
#ifdef _WIN32
        FreeLibrary(_hnd);
#else
        dlclose(_hnd);
#endif
    }
    */
}

IceInternal::DynamicLibrary::symbol_type
IceInternal::DynamicLibrary::loadEntryPoint(const string& entryPoint, bool useIceVersion)
{
    string::size_type colon = entryPoint.rfind(':');

#ifdef _WIN32
    const string driveLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if(colon == 1 && driveLetters.find(entryPoint[0]) != string::npos &&
       (entryPoint[2] == '\\' || entryPoint[2] == '/'))
    {
        //
        // The only colon we found is in the drive specification, as in "C:\MyDir".
        // This means the function name is missing.
        //
        colon = string::npos;
    }
#endif

    if(colon == string::npos || colon == entryPoint.size() - 1)
    {
        _err = "invalid entry point format `" + entryPoint + "'";
        return 0;
    }

    string libSpec = entryPoint.substr(0, colon);
    string funcName = entryPoint.substr(colon + 1);
    string libPath, libName, version, debug;

#ifdef _WIN32
    string::size_type separator = libSpec.find_last_of("/\\");
#else
    string::size_type separator = libSpec.rfind('/');
#endif
    if(separator != string::npos)
    {
        libPath = libSpec.substr(0, separator + 1);
        libSpec = libSpec.substr(separator + 1);
    }

    string::size_type comma = libSpec.find(',');
    if(comma == string::npos)
    {
        libName = libSpec;
#  ifdef ICE_LIBSUFFIX
        libName += ICE_LIBSUFFIX;
#  endif
        if(useIceVersion)
        {
            int majorVersion = (ICE_INT_VERSION / 10000);
            int minorVersion = (ICE_INT_VERSION / 100) - majorVersion * 100;
            ostringstream os;
            os << majorVersion * 10 + minorVersion;

            int patchVersion = ICE_INT_VERSION % 100;
            if(patchVersion > 50)
            {
                os << 'b';
                if(patchVersion >= 52)
                {
                    os << (patchVersion - 50);
                }
            }
            version = os.str();
        }
    }
    else
    {
        if(comma == libSpec.size() - 1)
        {
            _err = "invalid entry point format `" + entryPoint + "'";
            return 0;
        }
        libName = libSpec.substr(0, comma);
#ifdef ICE_LIBSUFFIX
        libName += ICE_LIBSUFFIX;
#endif
        version = libSpec.substr(comma + 1);
    }

    string lib = libPath;

#ifdef _WIN32
    lib += libName;
    lib += version;

#   if defined(_DEBUG) && !defined(__MINGW32__)
    lib += 'd';
#   endif

#   ifdef COMPSUFFIX
    lib += COMPSUFFIX;
#   endif

    lib += ".dll";
#elif defined(__APPLE__)
    lib += "lib" + libName;
    if(!version.empty())
    {
        lib += "." + version;
    }
#elif defined(__hpux)
    lib += "lib" + libName;
    if(!version.empty())
    {
        lib += "." + version;
    }
    else
    {
        lib += ".sl";
    }
#elif defined(_AIX)
    lib += "lib" + libName + ".a(lib" + libName + ".so";
    if(!version.empty())
    {
        lib += "." + version;
    }
    lib += ")";
#else
    lib += "lib" + libName + ".so";
    if(!version.empty())
    {
        lib += "." + version;
    }
#endif

#ifdef __APPLE__
    //
    // On OS X fallback to .so and .bundle extensions, if the default
    // .dylib fails.
    //
    if(!load(lib + ".dylib"))
    {
        string errMsg = _err;
        if(!load(lib + ".so"))
        {
            errMsg += "; " + _err;
            if(!load(lib + ".bundle"))
            {
                _err = errMsg + "; " + _err;
                return 0;
            }
        }
        _err = "";
    }
#else
    if(!load(lib))
    {
        return 0;
    }
#endif

    return getSymbol(funcName);
}

bool
IceInternal::DynamicLibrary::load(const string& lib)
{
    //
    // Don't need to use a wide string converter as the wide string is passed
    // to Windows API.
    //
#ifdef ICE_OS_WINRT
    _hnd = LoadPackagedLibrary(IceUtil::stringToWstring(lib, IceUtil::getProcessStringConverter()).c_str(), 0);
#elif defined(_WIN32)
    _hnd = LoadLibraryW(IceUtil::stringToWstring(lib, IceUtil::getProcessStringConverter()).c_str());
#else

    int flags = RTLD_NOW | RTLD_GLOBAL;
#ifdef _AIX
    flags |= RTLD_MEMBER;
#endif

    _hnd = dlopen(lib.c_str(), flags);
#endif
    if(_hnd == 0)
    {
        //
        // Remember the most recent error in _err.
        //
#ifdef _WIN32
        _err = IceUtilInternal::lastErrorToString();
#else
        const char* err = dlerror();
        if(err)
        {
            _err = err;
        }
#endif
    }

    return _hnd != 0;
}

IceInternal::DynamicLibrary::symbol_type
IceInternal::DynamicLibrary::getSymbol(const string& name)
{
    assert(_hnd != 0);
#ifdef _WIN32
    symbol_type result = GetProcAddress(_hnd, name.c_str());
#else
    symbol_type result = dlsym(_hnd, name.c_str());
#endif

    if(result == 0)
    {
        //
        // Remember the most recent error in _err.
        //
#ifdef _WIN32
        _err = IceUtilInternal::lastErrorToString();
#else
        const char* err = dlerror();
        if(err)
        {
            _err = err;
        }
#endif
    }
    return result;
}

const string&
IceInternal::DynamicLibrary::getErrorMessage() const
{
    return _err;
}

void
IceInternal::DynamicLibraryList::add(const DynamicLibraryPtr& library)
{
    _libraries.push_back(library);
}

