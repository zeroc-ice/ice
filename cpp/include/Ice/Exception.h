// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <Ice/Config.h>
#include <Ice/Handle.h>

//
// The manual forward declaration of ::Ice::Stream is necesary since
// Ice/StreamF.h includes Ice/Exception.h.
//
namespace Ice
{

class Stream;

}

namespace IceInternal
{

class BasicStream;

ICE_API void incRef(::Ice::Stream*);
ICE_API void decRef(::Ice::Stream*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::Stream> StreamPtr;

typedef IceUtil::Exception Exception;

class LocalException : public IceUtil::Exception
{
public:    

    ICE_API LocalException(const char*, int);
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

    virtual const std::string* __getExceptionIds() const = 0;
    virtual void __write(::IceInternal::BasicStream*) const = 0;
    virtual void __read(::IceInternal::BasicStream*) = 0;
    virtual void __marshal(const StreamPtr&) const = 0;
    virtual void __unmarshal(const StreamPtr&) = 0;

    void ice_marshal(const ::std::string&, const ::Ice::StreamPtr&);
};

}

#endif
