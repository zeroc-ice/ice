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

#ifndef ICE_USER_EXCEPTION_FACTORY_H
#define ICE_USER_EXCEPTION_FACTORY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <Ice/Config.h>

namespace Ice
{

class ICE_API UserExceptionFactory : public IceUtil::Shared
{
public:

    virtual void createAndThrow() = 0;
    virtual ~UserExceptionFactory() {}
};

typedef ::IceUtil::Handle<UserExceptionFactory> UserExceptionFactoryPtr;

};

#endif
