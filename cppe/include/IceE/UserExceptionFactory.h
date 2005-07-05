// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_USER_EXCEPTION_FACTORY_H
#define ICEE_USER_EXCEPTION_FACTORY_H

#include <IceE/Shared.h>
#include <IceE/Handle.h>
#include <IceE/Config.h>

namespace IceEInternal
{

class ICEE_API UserExceptionFactory : public IceE::Shared
{
public:

    virtual void createAndThrow() = 0;
    virtual ~UserExceptionFactory() {}
};

typedef ::IceE::Handle<UserExceptionFactory> UserExceptionFactoryPtr;

};

#endif
