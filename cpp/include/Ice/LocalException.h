// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCAL_EXCEPTION_H
#define ICE_LOCAL_EXCEPTION_H

#include <Ice/Config.h>

namespace Ice
{

struct ICE_API LocalException
{
    virtual ~LocalException();
    virtual std::string toString() const = 0;
    virtual LocalException* clone() const = 0;
};

std::ostream& operator<<(std::ostream&, const LocalException&);

}

#endif
