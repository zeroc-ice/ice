// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_UTIL_UUID_H
#define ICE_UTIL_UUID_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

namespace IceUtil
{

class ICE_UTIL_API UUIDGenerationException : public Exception
{
public:
    
    UUIDGenerationException(const char*, int);
    virtual const std::string& ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static ::std::string _name;
};

ICE_UTIL_API std::string generateUUID();

}

#endif
