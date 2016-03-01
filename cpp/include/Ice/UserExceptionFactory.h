// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

template<class E>
class DefaultUserExceptionFactory : public UserExceptionFactory
{
public:
    
    DefaultUserExceptionFactory(const ::std::string& typeId) :
        _typeId(typeId)
    {
    }

#ifndef NDEBUG
    virtual void createAndThrow(const ::std::string& typeId)
#else
    virtual void createAndThrow(const ::std::string&)
#endif
    {
        assert(typeId == _typeId);
        throw E();
    }

private:
    const ::std::string _typeId;
};

}

#endif
