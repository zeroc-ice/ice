// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include <IceUtil/Shared.h>
#include <Ice/ObjectF.h>
#include <Ice/Proxy.h>

namespace IceInternal
{

class Incoming;
class Stream;

enum DispatchStatus
{
    DispatchOK,
    DispatchException,
    DispatchLocationForward,
    DispatchObjectNotExist,
    DispatchOperationNotExist
};

}

namespace Ice
{

class ICE_API LocationForward
{
public:

    LocationForward(const LocationForward&);
    LocationForward(const ObjectPrx&);

protected:

    ObjectPrx _prx;
    friend class ::IceProxy::Ice::Object;
    friend class ::IceInternal::Incoming;
};

class ICE_API ObjectPtrE
{
public:

    ObjectPtrE() { }
    ObjectPtrE(const ObjectPtrE&);
    explicit ObjectPtrE(const ObjectPtr&);
    operator ObjectPtr() const;
    Object* operator->() const;
    operator bool() const;

protected:

    ObjectPtr _ptr;
};

class ICE_API Object : virtual public ::IceDelegate::Ice::Object
{
public:

    Object();
    virtual ~Object();

    virtual void _throw();
    virtual bool _isA(const std::string&);
    virtual void _ping();
    ::IceInternal::DispatchStatus ____isA(::IceInternal::Incoming&);
    ::IceInternal::DispatchStatus ____ping(::IceInternal::Incoming&);
    virtual const std::string* _classIds() = 0;

    static std::string __names[];
    static std::string __mutating[];
    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const std::string&);
    virtual bool __isMutating(const std::string&);

    virtual void __write(::IceInternal::Stream*) = 0;
    virtual void __read(::IceInternal::Stream*) = 0;
};

}

#endif
