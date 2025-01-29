// Copyright (c) ZeroC, Inc.

#include "ArgVector.h"
#include <cassert>
#include <cstring>

IceInternal::ArgVector::ArgVector(int argcP, const char* const argvP[])
{
    assert(argcP >= 0);
    _args.resize(static_cast<size_t>(argcP));
    for (size_t i = 0; i < static_cast<size_t>(argcP); ++i)
    {
        _args[i] = argvP[i];
    }
    setupArgcArgv();
}

IceInternal::ArgVector::ArgVector(const std::vector<std::string>& vec)
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
    if (this != &rhs)
    {
        delete[] argv;
        argv = nullptr;
        _args = rhs._args;
        setupArgcArgv();
    }
    return *this;
}

IceInternal::ArgVector::~ArgVector() { delete[] argv; }

void
IceInternal::ArgVector::setupArgcArgv()
{
    argc = static_cast<int>(_args.size());
    if ((argv = new char*[static_cast<size_t>(argc + 1)]) == nullptr)
    {
        throw std::bad_alloc();
    }
    for (size_t i = 0; i < static_cast<size_t>(argc); i++)
    {
        argv[i] = const_cast<char*>(_args[i].c_str());
    }
    argv[argc] = nullptr;
}
