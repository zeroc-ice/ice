// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

bool
IceInternal::DynamicLibrary::load(const string& lib)
{
#ifdef _WIN32
    _hnd = LoadLibrary(lib.c_str());
#else
    _hnd = dlopen(lib.c_str(), RTLD_NOW);
    if (_hnd == 0)
    {
        //
        // Remember the most recent error in _err.
        //
        const char* err = dlerror();
        if (err)
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
    if (result == 0)
    {
        //
        // Remember the most recent error in _err.
        //
        const char* err = dlerror();
        if (err)
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
