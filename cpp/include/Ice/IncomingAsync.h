// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_INCOMING_ASYNC_H
#define ICE_INCOMING_ASYNC_H

#include <Ice/IncomingAsyncF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ServantLocatorF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>

namespace IceInternal
{

class Incoming;

//
// We need virtual inheritance from shared, because we use multiple
// inheritance from IceUtil::Shared for generated AMD code.
//
class ICE_API IncomingAsync : virtual public ::IceUtil::Shared
{
public:

    IncomingAsync(Incoming&); // Adopts the Incoming argument. It must not be used afterwards.

protected:

    void __response(bool);
    void __exception(const ::Ice::Exception&);
    void __exception(const ::std::exception&);
    void __exception();

    BasicStream* __is();
    BasicStream* __os();

private:

    void finishInvoke();

    //
    // We need a separate InstancePtr, because _is and _os only hold a
    // const InstancePtr& for optimization.
    //
    InstancePtr _instance;

    Ice::Current _current;
    Ice::ObjectPtr _servant;
    Ice::ServantLocatorPtr _locator;
    Ice::LocalObjectPtr _cookie;

    ConnectionPtr _connection;

    bool _response;
    bool _compress;

    BasicStream _is;
    BasicStream _os;
};

}

namespace Ice
{

class ICE_API AMD_Object_ice_invoke : virtual public ::IceUtil::Shared
{
public:
    
    virtual void ice_response(bool, const ::std::vector< ::Ice::Byte>&) = 0;
    virtual void ice_exception(const ::IceUtil::Exception&) = 0;
    virtual void ice_exception(const ::std::exception&) = 0;
    virtual void ice_exception() = 0;
};

}

namespace IceAsync
{

namespace Ice
{

class ICE_API AMD_Object_ice_invoke : public ::Ice::AMD_Object_ice_invoke, public ::IceInternal::IncomingAsync
{
public:
    
    AMD_Object_ice_invoke(::IceInternal::Incoming&);
    
    virtual void ice_response(bool, const ::std::vector< ::Ice::Byte>&);
    virtual void ice_exception(const ::IceUtil::Exception&);
    virtual void ice_exception(const ::std::exception&);
    virtual void ice_exception();
};

}

}

#endif
