// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_EXCEPTION_H
#define ICEE_EXCEPTION_H

#include <IceE/ExceptionBase.h>
#include <IceE/Handle.h>

namespace IceInternal
{

class BasicStream;

}

namespace Ice
{

class ICEE_API LocalException : public Ice::Exception
{
public:    

    LocalException(const char*, int);
    virtual const std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;
};

class ICEE_API UserException : public Ice::Exception
{
public:    

    virtual const std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(::IceInternal::BasicStream*) const = 0;
    virtual void __read(::IceInternal::BasicStream*, bool) = 0;

    virtual bool __usesClasses() const;
};

typedef ::IceInternal::Handle<UserException> UserExceptionPtr;

}

#endif
