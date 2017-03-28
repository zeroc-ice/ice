// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ArgVector.h>
#include <cstring>

IceInternal::ArgVector::ArgVector(int argc, const char* const argv[])
{
    assert(argc >= 0);
    _args.resize(argc);
    for(int i = 0; i < argc; ++i)
    {
        _args[i] = argv[i];
    }
    setupArgcArgv();
}

IceInternal::ArgVector::ArgVector(const ::std::vector< ::std::string>& vec)
{
    _args = vec;
    setupArgcArgv();
}

IceInternal::ArgVector::ArgVector(const ArgVector& rhs)
{
    _args = rhs._args;
    setupArgcArgv();
}

IceInternal::ArgVector&
IceInternal::ArgVector::operator=(const ArgVector& rhs)
{
    delete[] argv;
    argv = 0;
    _args = rhs._args;
    setupArgcArgv();
    return *this;
}

IceInternal::ArgVector::~ArgVector()
{
    delete[] argv;
}

void
IceInternal::ArgVector::setupArgcArgv()
{
    argc = static_cast<int>(_args.size());
    if((argv = new char*[argc + 1]) == 0)
    {
        throw ::std::bad_alloc();
    }
    for(int i = 0; i < argc; i++)
    {
        argv[i] = const_cast<char*>(_args[i].c_str());
    }
    argv[argc] = 0;
}
