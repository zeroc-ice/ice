// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/DynamicLibrary.h>

#ifndef _WIN32
#   include <dlfcn.h>
#endif

using namespace Ice;
using namespace IceInternal;
using namespace std;

void IceInternal::incRef(DynamicLibrary* p) { p->__incRef(); }
void IceInternal::decRef(DynamicLibrary* p) { p->__decRef(); }

void IceInternal::incRef(DynamicLibraryList* p) { p->__incRef(); }
void IceInternal::decRef(DynamicLibraryList* p) { p->__decRef(); }

IceInternal::DynamicLibrary::DynamicLibrary()
    : _hnd(0)
{
}

IceInternal::DynamicLibrary::~DynamicLibrary()
{
    if(_hnd != 0)
    {
#ifdef _WIN32
        FreeLibrary(_hnd);
#else
        dlclose(_hnd);
#endif
    }
}

IceInternal::DynamicLibrary::symbol_type
IceInternal::DynamicLibrary::loadEntryPoint(const string& entryPoint)
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
        version = ICE_STRING_VERSION;
    }
    else
    {
        libName = libSpec.substr(0, comma);
        version = libSpec.substr(comma + 1);
    }

    string lib;

#ifdef _WIN32
    lib = libName;
    for(string::size_type n = 0; n < version.size(); n++)
    {
        if(version[n] != '.') // Remove periods
        {
            lib += version[n];
        }
    }
#   ifdef _DEBUG
    lib += 'd';
#   endif
    lib += ".dll";
#else
    lib = "lib" + libName + ".so." + version;
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
    _hnd = dlopen(lib.c_str(), RTLD_NOW);
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
    return GetProcAddress(_hnd, name.c_str());
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
