// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    virtual std::string ice_name() const
    {
        return "SystemFailure";
    }

    virtual SystemException* ice_clone() const
    {
        return new SystemFailure(*this);
    }

    virtual void ice_throw() const
    {
        throw SystemFailure(*this);
    }
};

#endif
