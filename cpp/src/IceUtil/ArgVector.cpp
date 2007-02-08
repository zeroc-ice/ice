// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ArgVector.h>
#include <IceUtil/DisableWarnings.h>

IceUtil::ArgVector::ArgVector(int argc, char *argv[])
{
    assert(argc >= 0);
    _noDelete = false;
    copyVec(argc, argc, argv);
}

IceUtil::ArgVector::ArgVector(const ::std::vector< ::std::string>& vec)
{
    _noDelete = false;
    copyVec(vec);
}

IceUtil::ArgVector::ArgVector(const ArgVector& rhs)
{
    _noDelete = false;
    copyVec(rhs.argc, rhs._origArgc, rhs.argv);
}

IceUtil::ArgVector&
IceUtil::ArgVector::operator=(const ArgVector& rhs)
{
    ArgVector tmp(rhs);
    swap(tmp);
    return *this;
}

IceUtil::ArgVector::~ArgVector()
{
    //
    // For use with putenv()--see man putenv.
    //
    if(!_noDelete)
    {
        for(int i = 0; i < _origArgc; ++i)
        {
            delete[] argv[i];
        }
    }
    delete[] argv;
}

void
IceUtil::ArgVector::setNoDelete()
{
    _noDelete = true;
}

void
IceUtil::ArgVector::copyVec(int argc, int origArgc, char** argv)
{
    this->argc = argc;
    this->_origArgc = origArgc;
    if((this->argv = new char*[argc + 1]) == 0)
    {
        throw ::std::bad_alloc();
    }
    for(int i = 0; i < argc; ++i)
    {
        try
        {
            if((this->argv[i] = new char[strlen(argv[i]) + 1]) == 0)
            {
                throw ::std::bad_alloc();
            }
        }
        catch(...)
        {
            for(int j = 0; j < i; ++j)
            {
                delete[] this->argv[j];
            }
            delete[] this->argv;
            throw;
        }
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
        strcpy_s(this->argv[i], strlen(argv[i]) + 1, argv[i]);
#else
        strcpy(this->argv[i], argv[i]);
#endif
    }
    this->argv[argc] = 0;
}

void
IceUtil::ArgVector::copyVec(const ::std::vector< ::std::string>& vec)
{
    argc = _origArgc = static_cast<int>(vec.size());
    if((argv = new char*[argc + 1]) == 0)
    {
        throw ::std::bad_alloc();
    }
    for(int i = 0; i < argc; ++i)
    {
        try
        {
            if((argv[i] = new char[vec[i].size() + 1]) == 0)
            {
                throw ::std::bad_alloc();
            }
        }
        catch(...)
        {
            for(int j = 0; j < i; ++j)
            {
                delete[] argv[j];
            }
            delete[] argv;
            throw;
        }
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
        strcpy_s(argv[i], vec[i].size() + 1, vec[i].c_str());
#else
        strcpy(argv[i], vec[i].c_str());
#endif
    }
    argv[argc] = 0;
}

void
IceUtil::ArgVector::swap(ArgVector& rhs) throw()
{
    int argcTmp = rhs.argc;
    int origArgcTmp = rhs._origArgc;
    char** argvTmp = rhs.argv;
    bool noDeleteTmp = rhs._noDelete;
    rhs.argc = argc;
    rhs._origArgc = _origArgc;
    rhs.argv = argv;
    rhs._noDelete = _noDelete;
    argc = argcTmp;
    _origArgc = origArgcTmp;
    argv = argvTmp;
    _noDelete = noDeleteTmp;
}
