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

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <Ice/Config.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class BasicStream;

}

namespace Ice
{

typedef IceUtil::Exception Exception;

class ICE_API LocalException : public IceUtil::Exception
{
public:    

    LocalException(const char*, int);
    virtual std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;
};

class ICE_API UserException : public IceUtil::Exception
{
public:    

    virtual std::string ice_name() const = 0;
    virtual Exception* ice_clone() const = 0;
    virtual void ice_throw() const = 0;

    virtual void __write(::IceInternal::BasicStream*) const = 0;
    virtual void __read(::IceInternal::BasicStream*, bool) = 0;

    virtual bool __usesClasses() const;
};

typedef ::IceInternal::Handle<UserException> UserExceptionPtr;

}

#endif
