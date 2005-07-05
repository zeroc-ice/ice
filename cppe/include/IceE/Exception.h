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

namespace IceEInternal
{

class BasicStream;

}

namespace IceE
{

class ICEE_API LocalException : public IceE::Exception
{
public:    

    LocalException(const char*, int);
    virtual const std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;
};

class ICEE_API UserException : public IceE::Exception
{
public:    

    virtual const std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(::IceEInternal::BasicStream*) const = 0;
    virtual void __read(::IceEInternal::BasicStream*, bool) = 0;

    virtual bool __usesClasses() const;
};

typedef ::IceEInternal::Handle<UserException> UserExceptionPtr;

}

#endif
