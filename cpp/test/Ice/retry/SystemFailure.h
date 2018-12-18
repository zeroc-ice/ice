// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef SYSTEMFAILURE_H
#define SYSTEMFAILURE_H

#include <Ice/Exception.h>

class SystemFailure : public Ice::SystemException
{
public:

    SystemFailure(const char* file, int line) : Ice::SystemException(file, line)
    {
    }

    SystemFailure(const SystemFailure& ex) : Ice::SystemException(ex.ice_file(), ex.ice_line())
    {
    }

    virtual std::string ice_id() const
    {
        return "SystemFailure";
    }

#ifdef ICE_CPP11_MAPPING
    virtual IceUtil::Exception* ice_cloneImpl() const
    {
        return new SystemFailure(*this);
    }
#else
    virtual SystemException* ice_clone() const
    {
        return new SystemFailure(*this);
    }
#endif

    virtual void ice_throw() const
    {
        throw SystemFailure(*this);
    }

};

#endif
