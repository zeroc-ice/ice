// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_USER_EXCEPTION_FACTORY_H
#define ICEE_USER_EXCEPTION_FACTORY_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>

namespace IceInternal
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
