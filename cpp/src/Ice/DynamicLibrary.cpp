// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/DynamicLibrary.h>

#ifndef _WIN32
#   include <dlfcn.h>
#endif

using namespace Ice;
using namespace IceInternal;
using namespace std;

IceUtil::Shared* IceInternal::upCast(DynamicLibrary* p) { return p; }
IceUtil::Shared* IceInternal::upCast(DynamicLibraryList* p) { return p; }

IceInternal::DynamicLibrary::DynamicLibrary()
    : _hnd(0)
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
    string::size_type comma = entryPoint.find(',');
    if(colon == string::npos || colon == entryPoint.size() - 1 ||
        (comma != string::npos && (comma > colon || comma == colon - 1)))
    {
        _err = "invalid entry point format `" + entryPoint + "'";
        return 0;
    }
    string libSpec = entryPoint.substr(0, colon);
    string funcName = entryPoint.substr(colon + 1);
    string libName, version, debug;
    if(comma == string::npos)
    {
        libName = libSpec;
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
        libName = libSpec.substr(0, comma);
        version = libSpec.substr(comma + 1);
    }

    string lib;

#ifdef _WIN32
    lib = libName + version;
#   ifdef _DEBUG
    lib += 'd';
#   endif
    lib += ".dll";
#elif defined(__APPLE__)
    lib = "lib" + libName;
    if(!version.empty()) 
    {
        lib += "." + version;
    }
    lib += ".dylib";
#elif defined(__hpux)
    lib = "lib" + libName;
    if(!version.empty())
    {
        lib += "." + version;
    }
    else
    {
        lib += ".sl";
    }
#elif defined(_AIX)
    lib = "lib" + libName + ".a(lib" + libName + ".so";
    if(!version.empty())
    {
        lib += "." + version;
    }
    lib += ")";
#else
    lib = "lib" + libName + ".so";
    if(!version.empty())
    {
        lib += "." + version;
    }
#endif

    if(!load(lib))
    {
        return 0;
    }

    return getSymbol(funcName);
}

bool
IceInternal::DynamicLibrary::load(const string& lib)
{
#ifdef _WIN32
    _hnd = LoadLibrary(lib.c_str());
#else

    int flags = RTLD_NOW | RTLD_GLOBAL;
#ifdef _AIX
    flags |= RTLD_MEMBER;
#endif

    _hnd = dlopen(lib.c_str(), flags);
    if(_hnd == 0)
    {
        //
        // Remember the most recent error in _err.
        //
        const char* err = dlerror();

        if(err)
        {
            _err = err;
        }
    }
#endif
    return _hnd != 0;
}

IceInternal::DynamicLibrary::symbol_type
IceInternal::DynamicLibrary::getSymbol(const string& name)
{
    assert(_hnd != 0);
#ifdef _WIN32
#  ifdef __BCPLUSPLUS__
    string newName = "_" + name;
    return GetProcAddress(_hnd, newName.c_str());
#  else
    return GetProcAddress(_hnd, name.c_str());
#  endif
#else
    symbol_type result = dlsym(_hnd, name.c_str());
    if(result == 0)
    {
        //
        // Remember the most recent error in _err.
        //
        const char* err = dlerror();
        if(err)
        {
            _err = err;
        }
    }
    return result;
#endif
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
