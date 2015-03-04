// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_USER_EXCEPTION_FACTORY_H
#define ICE_USER_EXCEPTION_FACTORY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <Ice/Config.h>

namespace IceInternal
{

class ICE_API UserExceptionFactory : public IceUtil::Shared
{
public:

    virtual void createAndThrow(const ::std::string&) = 0;
    virtual ~UserExceptionFactory() {}
};

typedef ::IceUtil::Handle<UserExceptionFactory> UserExceptionFactoryPtr;

}

#endif
