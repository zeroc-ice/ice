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

namespace __Ice
{

class Stream;

}

namespace Ice
{

class ICE_API LocalObject_ptrE
{
public:

    LocalObject_ptrE() { }
    explicit LocalObject_ptrE(const LocalObject_ptrE&);
    explicit LocalObject_ptrE(const LocalObject_ptr&);
    operator LocalObject_ptr() const;
    LocalObject* operator->() const;
    operator bool() const;

protected:

    LocalObject_ptr ptr_;        
};

class ICE_API LocalObject : public ::__Ice::Shared
{
public:

    LocalObject();
    virtual ~LocalObject();

    virtual void __write(::__Ice::Stream*) = 0;
    virtual void __read(::__Ice::Stream*) = 0;
};

}

#endif
