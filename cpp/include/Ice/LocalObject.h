// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCAL_OBJECT_H
#define ICE_LOCAL_OBJECT_H

#include <Ice/LocalObjectF.h>
#include <Ice/ProxyF.h> // TODO...
#include <Ice/Shared.h>

namespace IceInternal
{

class Stream;

}

namespace Ice
{

class ICE_API LocalObjectPtrE
{
public:

    LocalObjectPtrE() { }
    explicit LocalObjectPtrE(const LocalObjectPtrE&);
    explicit LocalObjectPtrE(const LocalObjectPtr&);
    operator LocalObjectPtr() const;
    LocalObject* operator->() const;
    operator bool() const;

protected:

    LocalObjectPtr _ptr;        
};

class ICE_API LocalObject : public ::IceInternal::Shared
{
public:

    LocalObject();
    virtual ~LocalObject();

    virtual void __write(::IceInternal::Stream*) = 0;
    virtual void __read(::IceInternal::Stream*) = 0;
};

}

#endif
