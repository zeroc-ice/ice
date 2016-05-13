// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ArgVector.h>
#include <cstring>

IceUtilInternal::ArgVector::ArgVector(int argc, char* const argv[])
{
    assert(argc >= 0);
    _args.resize(argc);
    for(int i = 0; i < argc; ++i)
    {
        _args[i] = argv[i];
    }
    setupArgcArgv();
}

IceUtilInternal::ArgVector::ArgVector(const ::std::vector< ::std::string>& vec)
{
    _args = vec;
    setupArgcArgv();
}

IceUtilInternal::ArgVector::ArgVector(const ArgVector& rhs)
{
    _args = rhs._args;
    setupArgcArgv();
}

IceUtilInternal::ArgVector&
IceUtilInternal::ArgVector::operator=(const ArgVector& rhs)
{
    delete[] argv;
    argv = 0;
    _args = rhs._args;
    setupArgcArgv();
    return *this;
}

IceUtilInternal::ArgVector::~ArgVector()
{
    delete[] argv;
}

void
IceUtilInternal::ArgVector::setupArgcArgv()
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
