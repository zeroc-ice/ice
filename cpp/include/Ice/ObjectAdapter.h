// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Generated from file `ObjectAdapter.ice'

#ifndef __ObjectAdapter_h__
#define __ObjectAdapter_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/LocalObjectF.h>
#include <Ice/Native.h>
#include <Ice/LocalObject.h>
#include <Ice/CommunicatorF.h>

namespace Ice
{

class ObjectAdapter;

}

namespace __Ice
{

void ICE_API incRef(::Ice::ObjectAdapter*);
void ICE_API decRef(::Ice::ObjectAdapter*);

}

namespace Ice
{

typedef ::__Ice::Handle< ::Ice::ObjectAdapter> ObjectAdapter_ptr;

}

namespace Ice
{

class ICE_API ObjectAdapter : public ::Ice::LocalObject
{
public: 

    virtual ::std::string getName() = 0;

    virtual ::Ice::Communicator_ptr getCommunicator() = 0;

    virtual void activate() = 0;

    virtual void hold() = 0;

    virtual void deactivate() = 0;

    virtual void add(const ::Ice::Object_ptr&, const ::std::string&) = 0;

    virtual void remove(const ::std::string&) = 0;

    virtual ::Ice::Object_ptr identityToObject(const ::std::string&) = 0;

    virtual ::std::string objectToIdentity(const ::Ice::Object_ptr&) = 0;

    virtual ::Ice::Object_ptr proxyToObject(const ::Ice::Object_prx&) = 0;

    virtual ::Ice::Object_prx objectToProxy(const ::Ice::Object_ptr&) = 0;

    virtual ::Ice::Object_prx identityToProxy(const ::std::string&) = 0;

    virtual ::std::string proxyToIdentity(const ::Ice::Object_prx&) = 0;

    virtual void __write(::__Ice::Stream*);
    virtual void __read(::__Ice::Stream*);
};

}

#endif
