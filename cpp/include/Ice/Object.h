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

#include <Ice/ObjectF.h>
#include <Ice/Proxy.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Incoming;
class Stream;

enum DispatchStatus
{
    DispatchOK,
    DispatchObjectNotExist,
    DispatchOperationNotExist,
    DispatchException
};

}

namespace Ice
{

class ICE_API Object_ptrE
{
public:

    Object_ptrE() { }
    explicit Object_ptrE(const Object_ptrE&);
    explicit Object_ptrE(const Object_ptr&);
    operator Object_ptr() const;
    Object* operator->() const;
    operator bool() const;

protected:

    Object_ptr ptr_;        
};

class ICE_API Object : virtual public ::__IceDelegate::Ice::Object
{
public:

    Object();
    virtual ~Object();

    virtual bool _isA(const std::string&);
    ::__Ice::DispatchStatus ____isA(::__Ice::Incoming&);
    virtual const std::string* _classIds() = 0;

    static std::string __names[];
    virtual ::__Ice::DispatchStatus __dispatch(::__Ice::Incoming&,
					       const std::string&);

    virtual void __write(::__Ice::Stream*) = 0;
    virtual void __read(::__Ice::Stream*) = 0;
};

}

#endif
