// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Generated from file `Communicator.ice'

#ifndef __Communicator_h__
#define __Communicator_h__

#include <Ice/LocalObject.h>
#include <Ice/LoggerF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PicklerF.h>
#include <Ice/PropertiesF.h>
#include <Ice/ValueFactoryF.h>

namespace Ice
{

class Communicator;

}

namespace __Ice
{

void ICE_API incRef(::Ice::Communicator*);
void ICE_API decRef(::Ice::Communicator*);

}

namespace Ice
{

typedef ::__Ice::Handle< ::Ice::Communicator> Communicator_ptr;

}

namespace Ice
{

class ICE_API Communicator : public ::Ice::LocalObject
{
public: 

    virtual void destroy() = 0;

    virtual void shutdown() = 0;

    virtual void waitForShutdown() = 0;

    virtual ::Ice::Object_prx stringToProxy(const ::std::string&) = 0;

    virtual ::Ice::ObjectAdapter_ptr createObjectAdapter(const ::std::string&) = 0;

    virtual ::Ice::ObjectAdapter_ptr createObjectAdapterWithEndpoints(const ::std::string&, const ::std::string&) = 0;

    virtual void installValueFactory(const ::Ice::ValueFactory_ptr&, const ::std::string&) = 0;

    virtual ::Ice::Properties_ptr getProperties() = 0;

    virtual ::Ice::Logger_ptr getLogger() = 0;

    virtual void setLogger(const ::Ice::Logger_ptr&) = 0;

    virtual ::Ice::Pickler_ptr getPickler() = 0;

    virtual void __write(::__Ice::Stream*);
    virtual void __read(::__Ice::Stream*);
};

}

#endif
