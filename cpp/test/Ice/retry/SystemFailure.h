//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    virtual IceUtil::Exception* ice_cloneImpl() const
    {
        return new SystemFailure(*this);
    }

    virtual void ice_throw() const
    {
        throw SystemFailure(*this);
    }

};

#endif
